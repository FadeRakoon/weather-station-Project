module.exports = {
  root: true,
  env: {
    node: true,
    browser: true,
  },
  globals: {
    Paho: 'readonly',
  },
  extends: [
    'plugin:vue/vue3-essential',
    'eslint:recommended',
  ],
  ignorePatterns: ['**/paho-mqtt-min.js'],
}