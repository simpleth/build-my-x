#!/usr/bin/env node
/**
 * java-cli Test Suite
 * Integration tests for the CLI tool
 */

const fs = require('fs-extra');
const path = require('path');
const os = require('os');
const { execSync } = require('child_process');

const SCRIPT_DIR = __dirname;
const CLI_DIR = path.resolve(SCRIPT_DIR, '..');
const CLI_BIN = path.join(CLI_DIR, 'bin', 'java-cli.js');
const TEST_OUTPUT = path.join(SCRIPT_DIR, '..', 'test-temp');
const PASS = '\x1b[32mPASS\x1b[0m';
const FAIL = '\x1b[31mFAIL\x1b[0m';
const SKIP = '\x1b[33mSKIP\x1b[0m';

let passed = 0;
let failed = 0;
let skipped = 0;

function skip(message) {
  console.log(`  ${SKIP} ${message}`);
  skipped++;
}

function runCli(args, cwd) {
  return execSync(`node "${CLI_BIN}" ${args}`, {
    cwd: cwd || TEST_OUTPUT,
    encoding: 'utf-8',
    stdio: 'pipe',
    shell: os.platform() === 'win32',
  });
}

function assert(condition, message) {
  if (condition) {
    console.log(`  ${PASS} ${message}`);
    passed++;
  } else {
    console.log(`  ${FAIL} ${message}`);
    failed++;
  }
}

function assertThrows(fn, message) {
  try {
    fn();
    console.log(`  ${FAIL} ${message} (expected error but none thrown)`);
    failed++;
  } catch {
    console.log(`  ${PASS} ${message}`);
    passed++;
  }
}

function section(title) {
  console.log(`\n${'='.repeat(40)}`);
  console.log(`  ${title}`);
  console.log(`${'='.repeat(40)}`);
}

// Setup: clean test output directory
fs.removeSync(TEST_OUTPUT);
fs.ensureDirSync(TEST_OUTPUT);

// ============================================================
section('Test 1: CLI meta commands (--help, --version)');

// Test --help
const helpOutput = runCli('--help');
assert(helpOutput.includes('init'), '--help shows init command');
assert(helpOutput.includes('build'), '--help shows build command');
assert(helpOutput.includes('sdk'), '--help shows sdk command');

// Test --version
const versionOutput = runCli('--version');
const pkg = require(path.join(CLI_DIR, 'package.json'));
assert(versionOutput.includes(pkg.version), `--version shows ${pkg.version}`);

// ============================================================
section('Test 2: java-cli init (project scaffolding)');

const projectDir = path.join(TEST_OUTPUT, 'test-project');

// Test init
runCli('init test-project', TEST_OUTPUT);
assert(fs.existsSync(projectDir), 'Project directory created');
assert(fs.existsSync(path.join(projectDir, 'java-cli.json')), 'Project config created');
assert(fs.existsSync(path.join(projectDir, 'src', 'Main.java')), 'Main.java created');
assert(fs.existsSync(path.join(projectDir, '.gitignore')), '.gitignore created');

// Test config content
const config = fs.readJsonSync(path.join(projectDir, 'java-cli.json'));
assert(config.name === 'test-project', 'Config has correct project name');
assert(config.mainClass === 'Main', 'Config has default mainClass');
assert(config.sourceDir === 'src', 'Config has default sourceDir');

// Test Main.java placeholder replacement
const mainContent = fs.readFileSync(path.join(projectDir, 'src', 'Main.java'), 'utf-8');
assert(mainContent.includes('Hello, test-project!'), 'Template placeholder replaced');

// Check if JDK is available for build/run tests
function hasJdk() {
  try {
    const javac = os.platform() === 'win32' ? 'javac.exe' : 'javac';
    execSync(`${javac} -version`, { stdio: 'pipe', shell: true, encoding: 'utf-8' });
    return true;
  } catch {
    return false;
  }
}

// ============================================================
section('Test 3: java-cli build (compilation)');

if (!hasJdk()) {
  skip('javac not available — skipping build test');
  skip('Build directory check skipped');
  skip('Compiled .class files check skipped');
} else {
  const buildOutput = runCli('build', projectDir);
  assert(buildOutput.includes('Build completed'), 'Build succeeds');

  const buildDir = path.join(projectDir, 'build');
  assert(fs.existsSync(buildDir), 'Build directory created');
  const classFiles = fs.readdirSync(buildDir)
    .filter(f => f.endsWith('.class'));
  assert(classFiles.length > 0, 'Compiled .class files exist');
}

// ============================================================
section('Test 4: java-cli run (execution)');

if (!hasJdk()) {
  skip('java not available — skipping run test');
} else {
  const runOutput = runCli('run', projectDir);
  assert(runOutput.includes('Hello, test-project!'), 'Run outputs expected greeting');
}

// ============================================================
section('Test 5: java-cli clean');

if (!hasJdk()) {
  skip('java not available — skipping clean test');
} else {
  const buildDir = path.join(projectDir, 'build');
  runCli('clean', projectDir);
  assert(!fs.existsSync(buildDir), 'Clean removes build directory');
}

// ============================================================
section('Test 6: java-cli run after clean');

if (!hasJdk()) {
  skip('java not available — skipping run-after-clean test');
} else {
  const runAfterClean = runCli('run', projectDir);
  assert(runAfterClean.includes('Hello, test-project!'), 'Run after clean auto-rebuilds');
}

// ============================================================
section('Test 6: java-cli sdk commands');

// Test sdk list (should not crash even with no SDKs installed)
const sdkListOutput = runCli('sdk list', TEST_OUTPUT);
assert(typeof sdkListOutput === 'string', 'sdk list runs without error');

// Test sdk remove with nonexistent version
assertThrows(() => {
  execSync(`node "${CLI_BIN}" sdk remove nonexistent-999`, {
    cwd: TEST_OUTPUT, encoding: 'utf-8', stdio: 'pipe', shell: os.platform() === 'win32',
  });
}, 'sdk remove fails gracefully on missing version');

// ============================================================
section('Test 7: Edge cases');

// Init in non-empty directory
const dirtyDir = path.join(TEST_OUTPUT, 'dirty');
fs.ensureDirSync(dirtyDir);
fs.writeFileSync(path.join(dirtyDir, 'somefile.txt'), 'data');
assertThrows(() => {
  runCli('init dirty', TEST_OUTPUT);
}, 'Init fails in non-empty directory');

// Build in non-project directory
assertThrows(() => {
  runCli('build', TEST_OUTPUT);
}, 'Build fails outside project directory');

// ============================================================
// Summary
// ============================================================
console.log(`\n${'='.repeat(40)}`);
console.log(`  Results: ${passed} passed, ${failed} failed, ${skipped} skipped`);
console.log(`${'='.repeat(40)}\n`);

if (failed > 0) process.exit(1);
