import { WebPlugin } from './capacitor-core.js';
const errorString = 'The plugin is not available on this platform';
const errorFn = Promise.reject(errorString);
export class ZeroConfWeb extends WebPlugin {
    getHostname() {
        return errorFn;
    }
    // eslint-disable-next-line @typescript-eslint/no-unused-vars
    register(_request) {
        return errorFn;
    }
    // eslint-disable-next-line @typescript-eslint/no-unused-vars
    unregister(_request) {
        return errorFn;
    }
    stop() {
        return errorFn;
    }
    watch(
    // eslint-disable-next-line @typescript-eslint/no-unused-vars
    _request, 
    // eslint-disable-next-line @typescript-eslint/no-unused-vars
    _callback) {
        return errorFn;
    }
    // eslint-disable-next-line @typescript-eslint/no-unused-vars
    unwatch(_request) {
        return errorFn;
    }
    close() {
        return errorFn;
    }
}
