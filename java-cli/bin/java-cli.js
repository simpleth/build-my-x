#!/usr/bin/env node

const { Command } = require('commander');
const chalk = require('chalk');
const path = require('path');
const fs = require('fs-extra');

const pkg = require('../package.json');
const { initProject } = require('../lib/project');
const { buildProject, cleanProject, runProject } = require('../lib/build');
const { listSdks, installSdk, removeSdk, useSdk, currentSdk, resolveJdkForProject } = require('../lib/sdk');
const { getSdkDir, setSdkDir, readConfig, writeConfig } = require('../lib/utils');

const program = new Command();

program
  .name('java-cli')
  .description('Cross-platform Java project scaffolding CLI')
  .version(pkg.version);

program
  .command('init <project-name>')
  .description('Create a new Java project')
  .option('-t, --template <name>', 'Template to use', 'default')
  .option('-d, --dir <path>', 'Target directory')
  .action(async (projectName, options) => {
    try {
      const targetDir = path.resolve(options.dir || process.cwd(), projectName);
      await initProject(projectName, targetDir, options.template);
      console.log(chalk.green(`\nProject "${projectName}" created at ${targetDir}`));

      const sdkVersion = await resolveJdkForProject();
      if (sdkVersion) {
        const configPath = path.join(targetDir, 'java-cli.json');
        const config = fs.readJsonSync(configPath);
        config.sdkVersion = sdkVersion;
        fs.writeJsonSync(configPath, config, { spaces: 2 });
      }

      console.log(chalk.cyan('\n  cd ' + projectName));
      console.log(chalk.cyan('  java-cli build'));
      console.log(chalk.cyan('  java-cli run'));
    } catch (err) {
      console.error(chalk.red('Error:'), err.message);
      process.exit(1);
    }
  });

program
  .command('build')
  .description('Compile the Java project in the current directory')
  .option('-o, --output <dir>', 'Output directory', 'build')
  .action(async (options) => {
    try {
      await buildProject(process.cwd(), options.output);
      console.log(chalk.green('Build completed.'));
    } catch (err) {
      console.error(chalk.red('Build failed:'), err.message);
      process.exit(1);
    }
  });

program
  .command('run [main-class]')
  .description('Run the compiled Java project')
  .option('-o, --output <dir>', 'Build output directory', 'build')
  .action(async (mainClass, options) => {
    try {
      await runProject(process.cwd(), options.output, mainClass);
    } catch (err) {
      console.error(chalk.red('Run failed:'), err.message);
      process.exit(1);
    }
  });

program
  .command('clean')
  .description('Clean build output')
  .action(async () => {
    try {
      await cleanProject(process.cwd());
      console.log(chalk.green('Cleaned.'));
    } catch (err) {
      console.error(chalk.red('Error:'), err.message);
      process.exit(1);
    }
  });

const sdkCmd = program.command('sdk').description('Manage Java SDK installations');

sdkCmd
  .command('list')
  .description('List installed Java SDKs')
  .action(async () => {
    try {
      const sdks = await listSdks();
      if (sdks.length === 0) {
        console.log(chalk.yellow('No SDKs installed. Use "java-cli sdk install <version>"'));
        return;
      }
      const current = await currentSdk();
      for (const sdk of sdks) {
        const marker = sdk.version === current ? chalk.green(' *') : '  ';
        console.log(`${marker} ${sdk.version}  ${chalk.gray(sdk.path)}`);
      }
    } catch (err) {
      console.error(chalk.red('Error:'), err.message);
      process.exit(1);
    }
  });

sdkCmd
  .command('install <version>')
  .description('Install a Java SDK version (e.g. 17, 21)')
  .action(async (version) => {
    try {
      console.log(chalk.cyan(`Installing Java SDK ${version}...`));
      await installSdk(version);
      console.log(chalk.green(`Java SDK ${version} installed.`));
    } catch (err) {
      console.error(chalk.red('Install failed:'), err.message);
      process.exit(1);
    }
  });

sdkCmd
  .command('remove <version>')
  .description('Remove an installed Java SDK version')
  .action(async (version) => {
    try {
      await removeSdk(version);
      console.log(chalk.green(`Java SDK ${version} removed.`));
    } catch (err) {
      console.error(chalk.red('Error:'), err.message);
      process.exit(1);
    }
  });

sdkCmd
  .command('use <version>')
  .description('Set the active Java SDK version')
  .action(async (version) => {
    try {
      await useSdk(version);
      console.log(chalk.green(`Switched to Java SDK ${version}`));
    } catch (err) {
      console.error(chalk.red('Error:'), err.message);
      process.exit(1);
    }
  });

sdkCmd
  .command('dir [path]')
  .description('Show or set the SDK installation directory')
  .action(async (customPath) => {
    try {
      if (customPath) {
        const result = setSdkDir(customPath);
        console.log(chalk.green(`SDK directory set to: ${result}`));
      } else {
        const current = getSdkDir();
        const cfg = readConfig();
        console.log(chalk.cyan(`SDK directory: ${current}`));
        if (!cfg.sdkDir) {
          console.log(chalk.gray('  (default)'));
        }
      }
    } catch (err) {
      console.error(chalk.red('Error:'), err.message);
      process.exit(1);
    }
  });

sdkCmd
  .command('scan [dir]')
  .description('Add a directory to scan for JDKs, or list registered dirs')
  .option('-r, --remove <dir>', 'Remove a directory from the scan list')
  .action(async (dir, options) => {
    try {
      const cfg = readConfig();
      const extraDirs = cfg.extraSdkDirs || [];

      if (options.remove) {
        const absPath = path.resolve(options.remove);
        cfg.extraSdkDirs = extraDirs.filter((d) => d !== absPath);
        writeConfig(cfg);
        console.log(chalk.green(`Removed from scan list: ${absPath}`));
        return;
      }

      if (dir) {
        const absPath = path.resolve(dir);
        if (!extraDirs.includes(absPath)) {
          extraDirs.push(absPath);
          cfg.extraSdkDirs = extraDirs;
          writeConfig(cfg);
        }
        console.log(chalk.green(`Added to scan list: ${absPath}`));
        return;
      }

      if (extraDirs.length === 0) {
        console.log(chalk.gray('No custom scan directories configured.'));
      } else {
        console.log(chalk.cyan('Custom scan directories:'));
        extraDirs.forEach((d, i) => console.log(`  [${i + 1}] ${d}`));
      }
      console.log(chalk.gray('\nAuto-scanned locations: ~/.jdks (IDEA)'));
    } catch (err) {
      console.error(chalk.red('Error:'), err.message);
      process.exit(1);
    }
  });

const configCmd = program.command('config').description('Manage java-cli configuration');

configCmd
  .command('dir [path]')
  .description('Show or set the SDK installation directory')
  .action(async (customPath) => {
    try {
      if (customPath) {
        const result = setSdkDir(customPath);
        console.log(chalk.green(`SDK directory set to: ${result}`));
      } else {
        const current = getSdkDir();
        const cfg = readConfig();
        console.log(chalk.cyan(`SDK directory: ${current}`));
        if (!cfg.sdkDir) {
          console.log(chalk.gray('  (default)'));
        }
      }
    } catch (err) {
      console.error(chalk.red('Error:'), err.message);
      process.exit(1);
    }
  });

program.parse(process.argv);
