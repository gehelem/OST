// Vendored from npm capacitor-zeroconf@4.0.0 (MIT) - js/zeroconf.js's own
// doc comment has the full story on why this is vendored (no bundler,
// ostcore/UI3 loads everything as plain ES modules) rather than left in
// node_modules (android/'s own native build still needs it there too -
// npm install is NOT removed, this is purely for the web-app side).
import { registerPlugin } from './capacitor-core.js';
const ZeroConf = registerPlugin('ZeroConf', {
    web: () => import('./capacitor-zeroconf-web.js').then((m) => new m.ZeroConfWeb()),
    electron: () => window.CapacitorCustomPlatform.plugins.ZeroConf,
});
export * from './capacitor-zeroconf-definitions.js';
export { ZeroConf };
