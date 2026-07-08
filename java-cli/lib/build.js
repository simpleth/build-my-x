const path = require('path');
const fs = require('fs-extra');
const chalk = require('chalk');
const os = require('os');

const { getJavaBin, getJavaHome } = require('./sdk');
const { getProjectConfig } = require('./project');
const { runCommand } = require('./utils');

async function buildProject(cwd, outputDir) {
  const { root, config } = getProjectConfig(cwd);

  const sourceDir = path.join(root, config.sourceDir || 'src');
  const buildDir = path.join(root, outputDir);

  if (!fs.existsSync(sourceDir)) {
    throw new Error(`Source directory "${sourceDir}" not found.`);
  }

  fs.ensureDirSync(buildDir);

  const javaFiles = findAllJavaFiles(sourceDir);
  if (javaFiles.length === 0) {
    throw new Error('No .java files found.');
  }

  console.log(chalk.cyan(`Compiling ${javaFiles.length} source file(s)...`));

  const javacPath = resolveJavac(config);
  const classpath = buildClasspath(config, root);

  const filesArg = javaFiles.map((f) => `"${f}"`).join(' ');
  const cpArg = classpath.length > 0 ? `-cp "${classpath.join(path.delimiter)}"` : '';

  const cmd = `"${javacPath}" -d "${buildDir}" ${cpArg} ${filesArg}`;

  try {
    await runCommand(javacPath, [
      '-d', buildDir,
      ...(classpath.length > 0 ? ['-cp', classpath.join(path.delimiter)] : []),
      ...javaFiles,
    ], { cwd: root });
  } catch {
    throw new Error('Compilation failed. Check for errors above.');
  }

  console.log(chalk.gray(`  Output: ${buildDir}`));
}

async function runProject(cwd, outputDir, mainClass) {
  const { root, config } = getProjectConfig(cwd);

  const buildDir = path.join(root, outputDir);
  if (!fs.existsSync(buildDir)) {
    console.log(chalk.yellow('Build directory not found. Running "java-cli build" first...'));
    await buildProject(cwd, outputDir);
  }

  mainClass = mainClass || config.mainClass || 'Main';

  const javaPath = resolveJava(config);
  const classpath = [buildDir, ...buildClasspath(config, root)];

  console.log(chalk.cyan(`Running ${mainClass}...`));
  await runCommand(javaPath, ['-cp', classpath.join(path.delimiter), mainClass], { cwd: root });
}

async function cleanProject(cwd) {
  const { root, config } = getProjectConfig(cwd);
  const buildDir = path.join(root, config.outputDir || 'build');
  if (fs.existsSync(buildDir)) {
    fs.removeSync(buildDir);
  }
}

function findAllJavaFiles(dir) {
  const results = [];
  const entries = fs.readdirSync(dir, { withFileTypes: true });
  for (const entry of entries) {
    const fullPath = path.join(dir, entry.name);
    if (entry.isDirectory()) {
      results.push(...findAllJavaFiles(fullPath));
    } else if (entry.name.endsWith('.java')) {
      results.push(fullPath);
    }
  }
  return results;
}

function resolveJavac(config) {
  const sdkBin = getJavaBin(config.sdkVersion);
  if (sdkBin) return path.join(sdkBin, os.platform() === 'win32' ? 'javac.exe' : 'javac');
  return os.platform() === 'win32' ? 'javac.exe' : 'javac';
}

function resolveJava(config) {
  const sdkBin = getJavaBin(config.sdkVersion);
  if (sdkBin) return path.join(sdkBin, os.platform() === 'win32' ? 'java.exe' : 'java');
  return os.platform() === 'win32' ? 'java.exe' : 'java';
}

function buildClasspath(config, root) {
  const cp = [];
  if (config.classpath && Array.isArray(config.classpath)) {
    for (const entry of config.classpath) {
      cp.push(path.resolve(root, entry));
    }
  }
  return cp;
}

module.exports = {
  buildProject,
  runProject,
  cleanProject,
};
