const path = require('path');
const fs = require('fs-extra');
const https = require('https');
const os = require('os');
const { execSync } = require('child_process');
const tar = require('tar');
const chalk = require('chalk');

const { getSdkDir, readConfig, writeConfig, getPlatform, getArch, ask } = require('./utils');

const ADOPTIUM_API = 'https://api.adoptium.net/v3';

async function httpGet(url) {
  return new Promise((resolve, reject) => {
    https.get(url, { headers: { 'User-Agent': 'java-cli' } }, (res) => {
      if (res.statusCode >= 300 && res.statusCode < 400 && res.headers.location) {
        https.get(res.headers.location, { headers: { 'User-Agent': 'java-cli' } }, (redirectRes) => {
          let data = '';
          redirectRes.on('data', (chunk) => { data += chunk; });
          redirectRes.on('end', () => resolve(JSON.parse(data)));
        }).on('error', reject);
        return;
      }
      let data = '';
      res.on('data', (chunk) => { data += chunk; });
      res.on('end', () => resolve(JSON.parse(data)));
    }).on('error', reject);
  });
}

async function downloadFile(url, dest) {
  return new Promise((resolve, reject) => {
    const file = fs.createWriteStream(dest);
    https.get(url, { headers: { 'User-Agent': 'java-cli' } }, (res) => {
      if (res.statusCode >= 300 && res.statusCode < 400 && res.headers.location) {
        https.get(res.headers.location, { headers: { 'User-Agent': 'java-cli' } }, (redirectRes) => {
          redirectRes.pipe(file);
          file.on('finish', () => { file.close(); resolve(); });
        }).on('error', reject);
        return;
      }
      res.pipe(file);
      file.on('finish', () => { file.close(); resolve(); });
    }).on('error', reject);
  });
}

async function getReleaseInfo(version) {
  const featureVersion = version.replace(/[^0-9]/g, '');
  const platform = getPlatform();
  const arch = getArch();

  const url = `${ADOPTIUM_API}/assets/feature_releases/${featureVersion}/ga` +
    `?architecture=${arch}&image_type=jdk&jvm_impl=hotspot&os=${platform}&page=0&page_size=1` +
    `&project=jdk&sort_method=DEFAULT&sort_order=DESC&vendor=eclipse`;

  const releases = await httpGet(url);
  if (!releases || releases.length === 0) {
    throw new Error(`No JDK release found for version ${featureVersion} on ${platform}-${arch}`);
  }

  const binary = releases[0].binaries[0];
  return {
    version: featureVersion,
    packageUrl: binary.package.link,
    packageName: binary.package.name,
    checksum: binary.package.checksum,
    size: binary.package.size,
  };
}

async function listSdks() {
  const sdkDir = getSdkDir();
  if (!fs.existsSync(sdkDir)) return [];

  const entries = fs.readdirSync(sdkDir);
  const sdks = [];

  for (const entry of entries) {
    const sdkPath = path.join(sdkDir, entry);
    if (fs.statSync(sdkPath).isDirectory()) {
      sdks.push({ version: entry, path: sdkPath });
    }
  }

  return sdks;
}

async function currentSdk() {
  const config = readConfig();
  return config.activeSdk || null;
}

async function installSdk(version) {
  const sdkDir = getSdkDir();
  const installPath = path.join(sdkDir, version);

  if (fs.existsSync(installPath)) {
    throw new Error(`SDK version ${version} is already installed.`);
  }

  const info = await getReleaseInfo(version);
  const pkgName = info.packageName;
  const archivePath = path.join(sdkDir, pkgName);
  const isZip = pkgName.endsWith('.zip');
  const isTarGz = pkgName.endsWith('.tar.gz') || pkgName.endsWith('.tgz');

  console.log(`  Downloading from Adoptium...`);
  await downloadFile(info.packageUrl, archivePath);

  console.log(`  Extracting...`);
  const tmpDir = path.join(sdkDir, `.tmp_${version}`);
  fs.ensureDirSync(tmpDir);

  if (isZip) {
    const AdmZip = require('adm-zip');
    const zip = new AdmZip(archivePath);
    zip.extractAllTo(tmpDir, true);
  } else if (isTarGz) {
    await tar.x({ file: archivePath, cwd: tmpDir });
  } else {
    throw new Error(`Unsupported archive format: ${pkgName}`);
  }

  const tmpEntries = fs.readdirSync(tmpDir, { withFileTypes: true });
  const topDirs = tmpEntries.filter((e) => e.isDirectory());
  if (topDirs.length === 1) {
    fs.moveSync(path.join(tmpDir, topDirs[0].name), installPath, { overwrite: true });
  } else {
    fs.moveSync(tmpDir, installPath, { overwrite: true });
  }

  if (fs.existsSync(tmpDir)) fs.removeSync(tmpDir);
  fs.removeSync(archivePath);
}

async function removeSdk(version) {
  const sdkDir = getSdkDir();
  const installPath = path.join(sdkDir, version);
  if (!fs.existsSync(installPath)) {
    throw new Error(`SDK version ${version} is not installed.`);
  }

  const config = readConfig();
  if (config.activeSdk === version) {
    config.activeSdk = null;
    writeConfig(config);
  }

  fs.removeSync(installPath);
}

async function useSdk(version) {
  const sdkDir = getSdkDir();
  const installPath = path.join(sdkDir, version);
  if (!fs.existsSync(installPath)) {
    throw new Error(`SDK version ${version} is not installed. Install it first.`);
  }

  const config = readConfig();
  config.activeSdk = version;
  writeConfig(config);
}

function getJavaHome(version) {
  if (!version) {
    const config = readConfig();
    version = config.activeSdk;
  }
  if (!version) return null;

  if (path.isAbsolute(version) || version.includes('\\') || version.includes('/')) {
    const binDir = path.join(version, 'bin');
    return fs.existsSync(binDir) ? version : null;
  }

  const installPath = path.join(getSdkDir(), version);
  if (fs.existsSync(installPath)) {
    if (getPlatform() === 'macos' && fs.existsSync(path.join(installPath, 'Contents', 'Home'))) {
      return path.join(installPath, 'Contents', 'Home');
    }
    return installPath;
  }
  return null;
}

function getJavaBin(version) {
  const home = getJavaHome(version);
  if (!home) return null;
  return path.join(home, 'bin');
}

function detectSystemJdk() {
  try {
    const javac = os.platform() === 'win32' ? 'javac.exe' : 'javac';
    const out = execSync(`"${javac}" -version`, { stdio: 'pipe', shell: true, encoding: 'utf-8' });
    const match = out.match(/(\d+)/);
    return match ? { version: match[1], type: 'system' } : null;
  } catch {
    return null;
  }
}

function detectJdkFromPath(jdkPath) {
  const javacFile = os.platform() === 'win32' ? 'javac.exe' : 'javac';
  const javacPath = path.join(jdkPath, 'bin', javacFile);
  if (!fs.existsSync(javacPath)) return null;

  let version = path.basename(jdkPath);
  const releaseFile = path.join(jdkPath, 'release');
  if (fs.existsSync(releaseFile)) {
    const content = fs.readFileSync(releaseFile, 'utf-8');
    const match = content.match(/JAVA_VERSION="([^"]+)"/);
    if (match) {
      const parts = match[1].split('.');
      version = parts[0] === '1' ? (parts[1] || '8') : parts[0];
    }
  } else {
    const verMatch = version.match(/(\d+)/);
    if (verMatch) version = verMatch[1];
  }

  return { version, path: jdkPath, type: 'external' };
}

function findJdksInDir(parentDir) {
  const results = [];
  if (!fs.existsSync(parentDir)) return results;

  const entries = fs.readdirSync(parentDir, { withFileTypes: true });
  for (const entry of entries) {
    if (!entry.isDirectory()) continue;
    const jdkInfo = detectJdkFromPath(path.join(parentDir, entry.name));
    if (jdkInfo) results.push(jdkInfo);
  }

  return results;
}

async function detectAllJdks() {
  const seen = new Set();
  const available = [];

  const sys = detectSystemJdk();
  if (sys) { seen.add('system'); available.push({ version: sys.version, type: 'system', label: `System JDK ${sys.version}` }); }

  const sdks = await listSdks();
  for (const sdk of sdks) {
    const key = `managed:${sdk.version}`;
    if (seen.has(key)) continue;
    seen.add(key);
    available.push({ version: sdk.version, type: 'managed', path: sdk.path, label: `java-cli ${sdk.version}` });
  }

  const ideaDir = path.join(os.homedir(), '.jdks');
  for (const jdk of findJdksInDir(ideaDir)) {
    if (seen.has(jdk.path)) continue;
    seen.add(jdk.path);
    available.push({ version: jdk.version, type: 'external', path: jdk.path, label: `IDEA JDK ${jdk.version}` });
  }

  const config = readConfig();
  const extraDirs = config.extraSdkDirs || [];
  for (const dir of extraDirs) {
    for (const jdk of findJdksInDir(dir)) {
      if (seen.has(jdk.path)) continue;
      seen.add(jdk.path);
      available.push({ version: jdk.version, type: 'external', path: jdk.path, label: `External JDK ${jdk.version}` });
    }
  }

  return available;
}

async function resolveJdkForProject() {
  const available = await detectAllJdks();

  if (available.length === 0) {
    console.log(chalk.yellow('\nNo JDK found on this system.'));
    if (!process.stdin.isTTY) {
      console.log(chalk.yellow('Run "java-cli sdk install <version>" before "java-cli build".'));
      return null;
    }
    const answer = await ask(chalk.cyan('Would you like to install a JDK now? (y/N) '));
    if (answer.toLowerCase() !== 'y') {
      console.log(chalk.yellow('You can install one later with: java-cli sdk install <version>'));
      return null;
    }
    const version = await ask(chalk.cyan('JDK version to install (e.g. 21): '));
    if (!version) return null;
    try {
      await installSdk(version);
      return version;
    } catch (err) {
      console.log(chalk.red(`Install failed: ${err.message}`));
      return null;
    }
  }

  if (available.length === 1) {
    const jdk = available[0];
    console.log(chalk.cyan(`Using JDK: ${jdk.label}`));
    return jdk.type === 'external' ? jdk.path : jdk.version;
  }

  console.log(chalk.cyan('\nAvailable JDKs:'));
  available.forEach((jdk, i) => {
    console.log(`  [${i + 1}] ${jdk.label}`);
  });
  console.log(`  [${available.length + 1}] Install a new JDK`);

  const answer = await ask(chalk.cyan(`\nChoose a JDK for this project (1-${available.length + 1}): `));
  const idx = parseInt(answer, 10) - 1;

  if (idx === available.length) {
    const version = await ask(chalk.cyan('JDK version to install (e.g. 21): '));
    if (!version) return null;
    try {
      await installSdk(version);
      return version;
    } catch (err) {
      console.log(chalk.red(`Install failed: ${err.message}`));
      return null;
    }
  }

  if (idx >= 0 && idx < available.length) {
    const jdk = available[idx];
    return jdk.type === 'external' ? jdk.path : jdk.version;
  }

  return null;
}

module.exports = {
  listSdks,
  currentSdk,
  installSdk,
  removeSdk,
  useSdk,
  getJavaHome,
  getJavaBin,
  resolveJdkForProject,
};
