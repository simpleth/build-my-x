const os = require('os');
const path = require('path');
const fs = require('fs-extra');
const { execSync, spawn } = require('child_process');

function getPlatform() {
  const p = os.platform();
  if (p === 'win32') return 'windows';
  if (p === 'darwin') return 'macos';
  if (p === 'linux') return 'linux';
  return p;
}

function getArch() {
  const arch = os.arch();
  if (arch === 'x64') return 'x64';
  if (arch === 'arm64') return 'aarch64';
  return arch;
}

function getHomeDir() {
  return os.homedir();
}

function getJavaCliDir() {
  const dir = path.join(getHomeDir(), '.java-cli');
  fs.ensureDirSync(dir);
  return dir;
}

function getSdkDir() {
  const config = readConfig();
  if (config.sdkDir) {
    const dir = path.resolve(config.sdkDir);
    fs.ensureDirSync(dir);
    return dir;
  }
  const dir = path.join(getJavaCliDir(), 'sdks');
  fs.ensureDirSync(dir);
  return dir;
}

function setSdkDir(customPath) {
  const absPath = path.resolve(customPath);
  fs.ensureDirSync(absPath);
  const config = readConfig();
  config.sdkDir = absPath;
  writeConfig(config);
  return absPath;
}

function getConfigPath() {
  return path.join(getJavaCliDir(), 'config.json');
}

function readConfig() {
  const configPath = getConfigPath();
  if (!fs.existsSync(configPath)) {
    return { activeSdk: null };
  }
  return fs.readJsonSync(configPath);
}

function writeConfig(config) {
  fs.writeJsonSync(getConfigPath(), config, { spaces: 2 });
}

function findProjectRoot(startDir) {
  let dir = path.resolve(startDir);
  while (true) {
    const configPath = path.join(dir, 'java-cli.json');
    if (fs.existsSync(configPath)) return dir;
    const parent = path.dirname(dir);
    if (parent === dir) return null;
    dir = parent;
  }
}

function loadProjectConfig(projectDir) {
  const configPath = path.join(projectDir, 'java-cli.json');
  if (!fs.existsSync(configPath)) {
    throw new Error('No java-cli.json found. Run "java-cli init" first.');
  }
  return fs.readJsonSync(configPath);
}

function runCommand(cmd, args, options = {}) {
  return new Promise((resolve, reject) => {
    const onWindows = os.platform() === 'win32';
    let child;
    if (onWindows) {
      const quoted = args.map((a) => /[ \t]/.test(a) ? `"${a}"` : a);
      const cmdline = `"${cmd}" ${quoted.join(' ')}`;
      child = spawn(cmdline, [], {
        stdio: 'inherit',
        shell: true,
        ...options,
      });
    } else {
      child = spawn(cmd, args, {
        stdio: 'inherit',
        ...options,
      });
    }
    child.on('close', (code) => {
      if (code === 0) resolve();
      else reject(new Error(`Command "${cmd}" exited with code ${code}`));
    });
    child.on('error', reject);
  });
}

function runCommandSync(cmd, args, options = {}) {
  return execSync([cmd, ...args].join(' '), {
    stdio: 'pipe',
    shell: os.platform() === 'win32',
    encoding: 'utf-8',
    ...options,
  }).trim();
}

function whichJava() {
  try {
    return runCommandSync('java', ['-version'], { stdio: 'pipe' });
  } catch {
    return null;
  }
}

function whichJavac() {
  try {
    return runCommandSync('javac', ['-version'], { stdio: 'pipe' });
  } catch {
    return null;
  }
}

function ask(question) {
  const readline = require('readline');
  const rl = readline.createInterface({ input: process.stdin, output: process.stdout });
  return new Promise((resolve) => {
    rl.question(question, (answer) => {
      rl.close();
      resolve(answer.trim());
    });
  });
}

module.exports = {
  getPlatform,
  getArch,
  getHomeDir,
  getJavaCliDir,
  getSdkDir,
  setSdkDir,
  getConfigPath,
  readConfig,
  writeConfig,
  findProjectRoot,
  loadProjectConfig,
  runCommand,
  runCommandSync,
  whichJava,
  whichJavac,
  ask,
};
