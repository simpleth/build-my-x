const path = require('path');
const fs = require('fs-extra');
const chalk = require('chalk');

const { findProjectRoot } = require('./utils');

const PROJECT_CONFIG = {
  name: 'java-cli.json',
  schema: {
    name: '',
    version: '1.0.0',
    mainClass: 'Main',
    sourceDir: 'src',
    outputDir: 'build',
    sdkVersion: null,
    classpath: [],
  },
};

async function initProject(projectName, targetDir, template) {
  if (fs.existsSync(targetDir)) {
    const contents = fs.readdirSync(targetDir);
    if (contents.length > 0) {
      throw new Error(`Directory "${targetDir}" is not empty.`);
    }
  }

  const templateDir = path.join(__dirname, '..', 'templates', template);
  if (!fs.existsSync(templateDir)) {
    throw new Error(`Template "${template}" not found.`);
  }

  await fs.copy(templateDir, targetDir);

  const configPath = path.join(targetDir, PROJECT_CONFIG.name);
  const config = { ...PROJECT_CONFIG.schema, name: projectName };
  fs.writeJsonSync(configPath, config, { spaces: 2 });

  const packageName = projectName.replace(/[^a-zA-Z0-9.]/g, '.').toLowerCase();
  if (packageName && !packageName.startsWith('.')) {
    updatePackagePlaceholders(targetDir, packageName);
  }
}

function updatePackagePlaceholders(targetDir, packageName) {
  const walkDir = (dir) => {
    const entries = fs.readdirSync(dir, { withFileTypes: true });
    for (const entry of entries) {
      const fullPath = path.join(dir, entry.name);
      if (entry.isDirectory()) {
        walkDir(fullPath);
        continue;
      }
      if (entry.name.endsWith('.java') || entry.name.endsWith('.json') || entry.name.endsWith('.md')) {
        let content = fs.readFileSync(fullPath, 'utf-8');
        content = content.replace(/\{\{PACKAGE\}\}/g, packageName);
        content = content.replace(/\{\{PROJECT_NAME\}\}/g, path.basename(targetDir));
        fs.writeFileSync(fullPath, content, 'utf-8');
      }
    }
  };
  walkDir(targetDir);
}

function getProjectConfig(cwd) {
  const root = findProjectRoot(cwd);
  if (!root) {
    throw new Error('Not in a java-cli project. Run "java-cli init <name>" first.');
  }
  const configPath = path.join(root, PROJECT_CONFIG.name);
  return { root, config: fs.readJsonSync(configPath) };
}

module.exports = {
  initProject,
  getProjectConfig,
};
