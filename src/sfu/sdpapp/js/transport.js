
/**
 * Error produced when calling a method in an invalid state.
 */
class InvalidStateError extends Error
{
    constructor(message)
    {
        super(message);

        this.name = 'InvalidStateError';

        if (Error.hasOwnProperty('captureStackTrace')) // Just in V8.
            Error.captureStackTrace(this, InvalidStateError);
        else
            this.stack = (new Error(message)).stack;
    }
}



/////////////////////////////////////////////////////////////////////////////////////////////


var objectCreate = Object.create || objectCreatePolyfill
var objectKeys = Object.keys || objectKeysPolyfill
var bind = Function.prototype.bind || functionBindPolyfill

function EventEmitter() {
  if (!this._events || !Object.prototype.hasOwnProperty.call(this, '_events')) {
    this._events = objectCreate(null);
    this._eventsCount = 0;
  }

  this._maxListeners = this._maxListeners || undefined;
}
//module.exports = EventEmitter;

// Backwards-compat with node 0.10.x
EventEmitter.EventEmitter = EventEmitter;

EventEmitter.prototype._events = undefined;
EventEmitter.prototype._maxListeners = undefined;

// By default EventEmitters will print a warning if more than 10 listeners are
// added to it. This is a useful default which helps finding memory leaks.
var defaultMaxListeners = 10;

var hasDefineProperty;
try {
  var o = {};
  if (Object.defineProperty) Object.defineProperty(o, 'x', { value: 0 });
  hasDefineProperty = o.x === 0;
} catch (err) { hasDefineProperty = false }
if (hasDefineProperty) {
  Object.defineProperty(EventEmitter, 'defaultMaxListeners', {
    enumerable: true,
    get: function() {
      return defaultMaxListeners;
    },
    set: function(arg) {
      // check whether the input is a positive number (whose value is zero or
      // greater and not a NaN).
      if (typeof arg !== 'number' || arg < 0 || arg !== arg)
        throw new TypeError('"defaultMaxListeners" must be a positive number');
      defaultMaxListeners = arg;
    }
  });
} else {
  EventEmitter.defaultMaxListeners = defaultMaxListeners;
}

// Obviously not all Emitters should be limited to 10. This function allows
// that to be increased. Set to zero for unlimited.
EventEmitter.prototype.setMaxListeners = function setMaxListeners(n) {
  if (typeof n !== 'number' || n < 0 || isNaN(n))
    throw new TypeError('"n" argument must be a positive number');
  this._maxListeners = n;
  return this;
};

function $getMaxListeners(that) {
  if (that._maxListeners === undefined)
    return EventEmitter.defaultMaxListeners;
  return that._maxListeners;
}

EventEmitter.prototype.getMaxListeners = function getMaxListeners() {
  return $getMaxListeners(this);
};

// These standalone emit* functions are used to optimize calling of event
// handlers for fast cases because emit() itself often has a variable number of
// arguments and can be deoptimized because of that. These functions always have
// the same number of arguments and thus do not get deoptimized, so the code
// inside them can execute faster.
function emitNone(handler, isFn, self) {
  if (isFn)
    handler.call(self);
  else {
    var len = handler.length;
    var listeners = arrayClone(handler, len);
    for (var i = 0; i < len; ++i)
      listeners[i].call(self);
  }
}
function emitOne(handler, isFn, self, arg1) {
  if (isFn)
    handler.call(self, arg1);
  else {
    var len = handler.length;
    var listeners = arrayClone(handler, len);
    for (var i = 0; i < len; ++i)
      listeners[i].call(self, arg1);
  }
}
function emitTwo(handler, isFn, self, arg1, arg2) {
  if (isFn)
    handler.call(self, arg1, arg2);
  else {
    var len = handler.length;
    var listeners = arrayClone(handler, len);
    for (var i = 0; i < len; ++i)
      listeners[i].call(self, arg1, arg2);
  }
}
function emitThree(handler, isFn, self, arg1, arg2, arg3) {
  if (isFn)
    handler.call(self, arg1, arg2, arg3);
  else {
    var len = handler.length;
    var listeners = arrayClone(handler, len);
    for (var i = 0; i < len; ++i)
      listeners[i].call(self, arg1, arg2, arg3);
  }
}

function emitMany(handler, isFn, self, args) {
  if (isFn)
    handler.apply(self, args);
  else {
    var len = handler.length;
    var listeners = arrayClone(handler, len);
    for (var i = 0; i < len; ++i)
      listeners[i].apply(self, args);
  }
}

EventEmitter.prototype.emit = function emit(type) {
  var er, handler, len, args, i, events;
  var doError = (type === 'error');

  events = this._events;
  if (events)
    doError = (doError && events.error == null);
  else if (!doError)
    return false;

  // If there is no 'error' event listener then throw.
  if (doError) {
    if (arguments.length > 1)
      er = arguments[1];
    if (er instanceof Error) {
      throw er; // Unhandled 'error' event
    } else {
      // At least give some kind of context to the user
      var err = new Error('Unhandled "error" event. (' + er + ')');
      err.context = er;
      throw err;
    }
    return false;
  }

  handler = events[type];

  if (!handler)
    return false;

  var isFn = typeof handler === 'function';
  len = arguments.length;
  switch (len) {
      // fast cases
    case 1:
      emitNone(handler, isFn, this);
      break;
    case 2:
      emitOne(handler, isFn, this, arguments[1]);
      break;
    case 3:
      emitTwo(handler, isFn, this, arguments[1], arguments[2]);
      break;
    case 4:
      emitThree(handler, isFn, this, arguments[1], arguments[2], arguments[3]);
      break;
      // slower
    default:
      args = new Array(len - 1);
      for (i = 1; i < len; i++)
        args[i - 1] = arguments[i];
      emitMany(handler, isFn, this, args);
  }

  return true;
};

function _addListener(target, type, listener, prepend) {
  var m;
  var events;
  var existing;

  if (typeof listener !== 'function')
    throw new TypeError('"listener" argument must be a function');

  events = target._events;
  if (!events) {
    events = target._events = objectCreate(null);
    target._eventsCount = 0;
  } else {
    // To avoid recursion in the case that type === "newListener"! Before
    // adding it to the listeners, first emit "newListener".
    if (events.newListener) {
      target.emit('newListener', type,
          listener.listener ? listener.listener : listener);

      // Re-assign `events` because a newListener handler could have caused the
      // this._events to be assigned to a new object
      events = target._events;
    }
    existing = events[type];
  }

  if (!existing) {
    // Optimize the case of one listener. Don't need the extra array object.
    existing = events[type] = listener;
    ++target._eventsCount;
  } else {
    if (typeof existing === 'function') {
      // Adding the second element, need to change to array.
      existing = events[type] =
          prepend ? [listener, existing] : [existing, listener];
    } else {
      // If we've already got an array, just append.
      if (prepend) {
        existing.unshift(listener);
      } else {
        existing.push(listener);
      }
    }

    // Check for listener leak
    if (!existing.warned) {
      m = $getMaxListeners(target);
      if (m && m > 0 && existing.length > m) {
        existing.warned = true;
        var w = new Error('Possible EventEmitter memory leak detected. ' +
            existing.length + ' "' + String(type) + '" listeners ' +
            'added. Use emitter.setMaxListeners() to ' +
            'increase limit.');
        w.name = 'MaxListenersExceededWarning';
        w.emitter = target;
        w.type = type;
        w.count = existing.length;
        if (typeof console === 'object' && console.log) {
          console.log('%s: %s', w.name, w.message);
        }
      }
    }
  }

  return target;
}

EventEmitter.prototype.addListener = function addListener(type, listener) {
  return _addListener(this, type, listener, false);
};

EventEmitter.prototype.on = EventEmitter.prototype.addListener;

EventEmitter.prototype.prependListener =
    function prependListener(type, listener) {
      return _addListener(this, type, listener, true);
    };

function onceWrapper() {
  if (!this.fired) {
    this.target.removeListener(this.type, this.wrapFn);
    this.fired = true;
    switch (arguments.length) {
      case 0:
        return this.listener.call(this.target);
      case 1:
        return this.listener.call(this.target, arguments[0]);
      case 2:
        return this.listener.call(this.target, arguments[0], arguments[1]);
      case 3:
        return this.listener.call(this.target, arguments[0], arguments[1],
            arguments[2]);
      default:
        var args = new Array(arguments.length);
        for (var i = 0; i < args.length; ++i)
          args[i] = arguments[i];
        this.listener.apply(this.target, args);
    }
  }
}

function _onceWrap(target, type, listener) {
  var state = { fired: false, wrapFn: undefined, target: target, type: type, listener: listener };
  var wrapped = bind.call(onceWrapper, state);
  wrapped.listener = listener;
  state.wrapFn = wrapped;
  return wrapped;
}

EventEmitter.prototype.once = function once(type, listener) {
  if (typeof listener !== 'function')
    throw new TypeError('"listener" argument must be a function');
  this.on(type, _onceWrap(this, type, listener));
  return this;
};

EventEmitter.prototype.prependOnceListener =
    function prependOnceListener(type, listener) {
      if (typeof listener !== 'function')
        throw new TypeError('"listener" argument must be a function');
      this.prependListener(type, _onceWrap(this, type, listener));
      return this;
    };

// Emits a 'removeListener' event if and only if the listener was removed.
EventEmitter.prototype.removeListener =
    function removeListener(type, listener) {
      var list, events, position, i, originalListener;

      if (typeof listener !== 'function')
        throw new TypeError('"listener" argument must be a function');

      events = this._events;
      if (!events)
        return this;

      list = events[type];
      if (!list)
        return this;

      if (list === listener || list.listener === listener) {
        if (--this._eventsCount === 0)
          this._events = objectCreate(null);
        else {
          delete events[type];
          if (events.removeListener)
            this.emit('removeListener', type, list.listener || listener);
        }
      } else if (typeof list !== 'function') {
        position = -1;

        for (i = list.length - 1; i >= 0; i--) {
          if (list[i] === listener || list[i].listener === listener) {
            originalListener = list[i].listener;
            position = i;
            break;
          }
        }

        if (position < 0)
          return this;

        if (position === 0)
          list.shift();
        else
          spliceOne(list, position);

        if (list.length === 1)
          events[type] = list[0];

        if (events.removeListener)
          this.emit('removeListener', type, originalListener || listener);
      }

      return this;
    };

EventEmitter.prototype.removeAllListeners =
    function removeAllListeners(type) {
      var listeners, events, i;

      events = this._events;
      if (!events)
        return this;

      // not listening for removeListener, no need to emit
      if (!events.removeListener) {
        if (arguments.length === 0) {
          this._events = objectCreate(null);
          this._eventsCount = 0;
        } else if (events[type]) {
          if (--this._eventsCount === 0)
            this._events = objectCreate(null);
          else
            delete events[type];
        }
        return this;
      }

      // emit removeListener for all listeners on all events
      if (arguments.length === 0) {
        var keys = objectKeys(events);
        var key;
        for (i = 0; i < keys.length; ++i) {
          key = keys[i];
          if (key === 'removeListener') continue;
          this.removeAllListeners(key);
        }
        this.removeAllListeners('removeListener');
        this._events = objectCreate(null);
        this._eventsCount = 0;
        return this;
      }

      listeners = events[type];

      if (typeof listeners === 'function') {
        this.removeListener(type, listeners);
      } else if (listeners) {
        // LIFO order
        for (i = listeners.length - 1; i >= 0; i--) {
          this.removeListener(type, listeners[i]);
        }
      }

      return this;
    };

function _listeners(target, type, unwrap) {
  var events = target._events;

  if (!events)
    return [];

  var evlistener = events[type];
  if (!evlistener)
    return [];

  if (typeof evlistener === 'function')
    return unwrap ? [evlistener.listener || evlistener] : [evlistener];

  return unwrap ? unwrapListeners(evlistener) : arrayClone(evlistener, evlistener.length);
}

EventEmitter.prototype.listeners = function listeners(type) {
  return _listeners(this, type, true);
};

EventEmitter.prototype.rawListeners = function rawListeners(type) {
  return _listeners(this, type, false);
};

EventEmitter.listenerCount = function(emitter, type) {
  if (typeof emitter.listenerCount === 'function') {
    return emitter.listenerCount(type);
  } else {
    return listenerCount.call(emitter, type);
  }
};

EventEmitter.prototype.listenerCount = listenerCount;
function listenerCount(type) {
  var events = this._events;

  if (events) {
    var evlistener = events[type];

    if (typeof evlistener === 'function') {
      return 1;
    } else if (evlistener) {
      return evlistener.length;
    }
  }

  return 0;
}

EventEmitter.prototype.eventNames = function eventNames() {
  return this._eventsCount > 0 ? Reflect.ownKeys(this._events) : [];
};

// About 1.5x faster than the two-arg version of Array#splice().
function spliceOne(list, index) {
  for (var i = index, k = i + 1, n = list.length; k < n; i += 1, k += 1)
    list[i] = list[k];
  list.pop();
}

function arrayClone(arr, n) {
  var copy = new Array(n);
  for (var i = 0; i < n; ++i)
    copy[i] = arr[i];
  return copy;
}

function unwrapListeners(arr) {
  var ret = new Array(arr.length);
  for (var i = 0; i < ret.length; ++i) {
    ret[i] = arr[i].listener || arr[i];
  }
  return ret;
}

function objectCreatePolyfill(proto) {
  var F = function() {};
  F.prototype = proto;
  return new F;
}
function objectKeysPolyfill(obj) {
  var keys = [];
  for (var k in obj) if (Object.prototype.hasOwnProperty.call(obj, k)) {
    keys.push(k);
  }
  return k;
}
function functionBindPolyfill(context) {
  var fn = this;
  return function () {
    return fn.apply(context, arguments);
  };
}
/*
},{}],5:[function(require,module,exports){
exports.read = function (buffer, offset, isLE, mLen, nBytes) {
  var e, m
  var eLen = (nBytes * 8) - mLen - 1
  var eMax = (1 << eLen) - 1
  var eBias = eMax >> 1
  var nBits = -7
  var i = isLE ? (nBytes - 1) : 0
  var d = isLE ? -1 : 1
  var s = buffer[offset + i]

  i += d

  e = s & ((1 << (-nBits)) - 1)
  s >>= (-nBits)
  nBits += eLen
  for (; nBits > 0; e = (e * 256) + buffer[offset + i], i += d, nBits -= 8) {}

  m = e & ((1 << (-nBits)) - 1)
  e >>= (-nBits)
  nBits += mLen
  for (; nBits > 0; m = (m * 256) + buffer[offset + i], i += d, nBits -= 8) {}

  if (e === 0) {
    e = 1 - eBias
  } else if (e === eMax) {
    return m ? NaN : ((s ? -1 : 1) * Infinity)
  } else {
    m = m + Math.pow(2, mLen)
    e = e - eBias
  }
  return (s ? -1 : 1) * m * Math.pow(2, e - mLen)
}

exports.write = function (buffer, value, offset, isLE, mLen, nBytes) {
  var e, m, c
  var eLen = (nBytes * 8) - mLen - 1
  var eMax = (1 << eLen) - 1
  var eBias = eMax >> 1
  var rt = (mLen === 23 ? Math.pow(2, -24) - Math.pow(2, -77) : 0)
  var i = isLE ? 0 : (nBytes - 1)
  var d = isLE ? 1 : -1
  var s = value < 0 || (value === 0 && 1 / value < 0) ? 1 : 0

  value = Math.abs(value)

  if (isNaN(value) || value === Infinity) {
    m = isNaN(value) ? 1 : 0
    e = eMax
  } else {
    e = Math.floor(Math.log(value) / Math.LN2)
    if (value * (c = Math.pow(2, -e)) < 1) {
      e--
      c *= 2
    }
    if (e + eBias >= 1) {
      value += rt / c
    } else {
      value += rt * Math.pow(2, 1 - eBias)
    }
    if (value * c >= 2) {
      e++
      c /= 2
    }

    if (e + eBias >= eMax) {
      m = 0
      e = eMax
    } else if (e + eBias >= 1) {
      m = ((value * c) - 1) * Math.pow(2, mLen)
      e = e + eBias
    } else {
      m = value * Math.pow(2, eBias - 1) * Math.pow(2, mLen)
      e = 0
    }
  }

  for (; mLen >= 8; buffer[offset + i] = m & 0xff, i += d, m /= 256, mLen -= 8) {}

  e = (e << mLen) | m
  eLen += mLen
  for (; eLen > 0; buffer[offset + i] = e & 0xff, i += d, e /= 256, eLen -= 8) {}

  buffer[offset + i - d] |= s * 128
}
*/
/////////////////////////////////////////debug//////////////////////////////////////////
instances = [];
function createDebug(namespace) {

  var prevTime;

  function debug() {
    // disabled?
    // if (!debug.enabled) return;

    var self = debug;

    // set `diff` timestamp
    var curr = +new Date();
    var ms = curr - (prevTime || curr);
    self.diff = ms;
    self.prev = prevTime;
    self.curr = curr;
    prevTime = curr;

    // turn the `arguments` into a proper Array
    var args = new Array(arguments.length);
    for (var i = 0; i < args.length; i++) {
      args[i] = arguments[i];
    }

    //args[0] = exports.coerce(args[0]);

    if ('string' !== typeof args[0]) {
      // anything else let's inspect with %O
      args.unshift('%O');
    }

    // apply any `formatters` transformations
    var index = 0;
    args[0] = args[0].replace(/%([a-zA-Z%])/g, function(match, format) {
      // if we encounter an escaped % then don't increase the array index
      if (match === '%%') return match;
      index++;
      // var formatter = exports.formatters[format];
      // if ('function' === typeof formatter) {
      //   var val = args[index];
      //   match = formatter.call(self, val);

      //   // now we need to remove `args[index]` since it's inlined in the `format`
      //   args.splice(index, 1);
      //   index--;
      // }
      return match;
    });

    // // apply env-specific formatting (colors, etc.)
    // exports.formatArgs.call(self, args);

    var logFn = debug.log || console.log.bind(console);
    logFn.apply(self, args);
  }

  debug.namespace = namespace;
 // debug.enabled = exports.enabled(namespace);
 // debug.useColors = exports.useColors();
 // debug.color = selectColor(namespace);
  debug.destroy = destroy;

  // env-specific initialization logic for debug instances
  // if ('function' === typeof exports.init) {
  //   exports.init(debug);
  // }

  instances.push(debug);

  return debug;
}

function destroy () {
  var index = exports.instances.indexOf(this);
  if (index !== -1) {
    exports.instances.splice(index, 1);
    return true;
  } else {
    return false;
  }
}

/**
 * Enables a debug mode by namespaces. This can include modes
 * separated by a colon and wildcards.
 *
 * @param {String} namespaces
 * @api public
 */

function enable(namespaces) {
  exports.save(namespaces);

  exports.names = [];
  exports.skips = [];

  var i;
  var split = (typeof namespaces === 'string' ? namespaces : '').split(/[\s,]+/);
  var len = split.length;

  for (i = 0; i < len; i++) {
    if (!split[i]) continue; // ignore empty strings
    namespaces = split[i].replace(/\*/g, '.*?');
    if (namespaces[0] === '-') {
      exports.skips.push(new RegExp('^' + namespaces.substr(1) + '$'));
    } else {
      exports.names.push(new RegExp('^' + namespaces + '$'));
    }
  }

  for (i = 0; i < exports.instances.length; i++) {
    var instance = exports.instances[i];
    instance.enabled = exports.enabled(instance.namespace);
  }
}

/**
 * Disable debug output.
 *
 * @api public
 */

function disable() {
  exports.enable('');
}

/**
 * Returns true if the given mode name is enabled, false otherwise.
 *
 * @param {String} name
 * @return {Boolean}
 * @api public
 */

function enabled(name) {
  if (name[name.length - 1] === '*') {
    return true;
  }
  var i, len;
  for (i = 0, len = exports.skips.length; i < len; i++) {
    if (exports.skips[i].test(name)) {
      return false;
    }
  }
  for (i = 0, len = exports.names.length; i < len; i++) {
    if (exports.names[i].test(name)) {
      return true;
    }
  }
  return false;
}



//////////////////////////////////////debuger end
////////////////////////////////////////////////////////////////////////////////////////////

class AwaitQueue
{
  constructor({ ClosedErrorClass = Error } = {})
  {
    // Closed flag.
    // @type {Boolean}
    this._closed = false;

    // Queue of pending tasks. Each task is a function that returns a promise
    // or a value directly.
    // @type {Array<Function>}
    this._tasks = [];

    // Error used when rejecting a task after the AwaitQueue has been closed.
    // @type {Error}
    this._closedErrorClass = ClosedErrorClass;
  }

  close()
  {
    this._closed = true;
  }

  /**
   * @param {Function} task - Function that returns a promise or a value directly.
   *
   * @async
   */
  async push(task)
  {
    if (typeof task !== 'function')
      throw new TypeError('given task is not a function');

    return new Promise((resolve, reject) =>
    {
      task._resolve = resolve;
      task._reject = reject;

      // Append task to the queue.
      this._tasks.push(task);

      // And run it if the only task in the queue is the new one.
      if (this._tasks.length === 1)
        this._next();
    });
  }

  async _next()
  {
    // Take the first task.
    const task = this._tasks[0];

    if (!task)
      return;

    // Execute it.
    await this._runTask(task);

    // Remove the first task (the completed one) from the queue.
    this._tasks.shift();

    // And continue.
    this._next();
  }

  async _runTask(task)
  {
    if (this._closed)
    {
      task._reject(new this._closedErrorClass('AwaitQueue closed'));

      return;
    }

    try
    {
      const result = await task();

      if (this._closed)
      {
        task._reject(new this._closedErrorClass('AwaitQueue closed'));

        return;
      }

      // Resolve the task with the given result (if any).
      task._resolve(result);
    }
    catch (error)
    {
      if (this._closed)
      {
        task._reject(new this._closedErrorClass('AwaitQueue closed'));

        return;
      }

      // Reject the task with the error.
      task._reject(error);
    }
  }
}
/////////////////////////////////////////////////////////////////////////////////////

class EnhancedEventEmitter extends EventEmitter
{
    constructor()
    {
        super();
        this.setMaxListeners(Infinity);

    }

    safeEmit(event, ...args)
    {
        try
        {
            this.emit(event, ...args);
        }
        catch (error)
        {
            this._console.error(
                'safeEmit() | event listener threw an error [event:%s]:%o',
                event, error);
        }
    }

    async safeEmitAsPromise(event, ...args)
    {
        return new Promise((resolve, reject) =>
        {
            this.safeEmit(event, ...args, resolve, reject);
        });
    }
}




class Transport extends EnhancedEventEmitter
{
    /**
     * @private
     *
     * @emits {transportLocalParameters: Object, callback: Function, errback: Function} connect
     * @emits {producerLocalParameters: Object, callback: Function, errback: Function} produce
     * @emits {connectionState: String} connectionstatechange
     */
    constructor(
        {
            direction,
            id,
            iceParameters,
            iceCandidates,
            dtlsParameters,
            iceServers,
            iceTransportPolicy,
            proprietaryConstraints,
            appData,
            Handler,
            extendedRtpCapabilities,
            canProduceByKind
        }
    )
    {
        super();

        console.log('constructor() [id:%s, direction:%s]', id, direction);

        // Id.
        // @type {String}
        this._id = id;

        // Closed flag.
        // @type {Boolean}
        this._closed = false;

        // Direction.
        // @type {String}
        this._direction = direction;

        // Extended RTP capabilities.
        // @type {Object}
        this._extendedRtpCapabilities = extendedRtpCapabilities;

        // Whether we can produce audio/video based on computed extended RTP
        // capabilities.
        // @type {Object}
        this._canProduceByKind = canProduceByKind;

        // RTC handler instance.
        // @type {Handler}
        this._handler = new Handler(
            {
                direction,
                iceParameters,
                iceCandidates,
                dtlsParameters,
                iceServers,
                iceTransportPolicy,
                proprietaryConstraints,
                extendedRtpCapabilities
            });

        // Transport connection state. Values can be:
        // 'new'/'connecting'/'connected'/'failed'/'disconnected'/'closed'
        // @type {String}
        this._connectionState = 'new';

        // App custom data.
        // @type {Object}
        this._appData = appData;

        // Map of Producers indexed by id.
        // @type {Map<String, Producer>}
        this._producers = new Map();

        // Map of Consumers indexed by id.
        // @type {Map<String, Consumer>}
        this._consumers = new Map();

        // AwaitQueue instance to make async tasks happen sequentially.
        // @type {AwaitQueue}
        this._awaitQueue = new AwaitQueue({ ClosedErrorClass: InvalidStateError });

        this._handleHandler();
    }

    /**
     * Transport id.
     *
     * @returns {String}
     */
    get id()
    {
        return this._id;
    }

    /**
     * Whether the Transport is closed.
     *
     * @returns {Boolean}
     */
    get closed()
    {
        return this._closed;
    }

    /**
     * Transport direction.
     *
     * @returns {String}
     */
    get direction()
    {
        return this._direction;
    }

    /**
     * RTC handler instance.
     *
     * @returns {Handler}
     */
    get handler()
    {
        return this._handler;
    }

    /**
     * Connection state.
     *
     * @returns {String}
     */
    get connectionState()
    {
        return this._connectionState;
    }

    /**
     * App custom data.
     *
     * @returns {Object}
     */
    get appData()
    {
        return this._appData;
    }

    /**
     * Invalid setter.
     */
    set appData(appData) // eslint-disable-line no-unused-vars
    {
        throw new Error('cannot override appData object');
    }

    /**
     * Close the Transport.
     */
    close()
    {
        if (this._closed)
            return;

        console.log('close()');

        this._closed = true;

        // Close the AwaitQueue.
        this._awaitQueue.close();

        // Close the handler.
        this._handler.close();

        // Close all Producers.
        for (const producer of this._producers.values())
        {
            producer.transportClosed();
        }
        this._producers.clear();

        // Close all Consumers.
        for (const consumer of this._consumers.values())
        {
            consumer.transportClosed();
        }
        this._consumers.clear();
    }

    /**
     * Get associated Transport (RTCPeerConnection) stats.
     *
     * @async
     * @returns {RTCStatsReport}
     * @throws {InvalidStateError} if Transport closed.
     */
    async getStats()
    {
        if (this._closed)
            throw new InvalidStateError('closed');

        return this._handler.getTransportStats();
    }

    /**
     * Restart ICE connection.
     *
     * @param {RTCIceParameters} iceParameters - New Server-side Transport ICE parameters.
     *
     * @async
     * @throws {InvalidStateError} if Transport closed.
     * @throws {TypeError} if wrong arguments.
     */
    async restartIce({ iceParameters } = {})
    {
        console.log('restartIce()');

        if (this._closed)
            throw new InvalidStateError('closed');
        else if (!iceParameters)
            throw new TypeError('missing iceParameters');

        // Enqueue command.
        return this._awaitQueue.push(
            async () => this._handler.restartIce({ iceParameters }));
    }


    /**
     * Update ICE servers.
     *
     * @param {Array<RTCIceServer>} [iceServers] - Array of ICE servers.
     *
     * @async
     * @throws {InvalidStateError} if Transport closed.
     * @throws {TypeError} if wrong arguments.
     */
    async updateIceServers({ iceServers } = {})
    {
        console.log('updateIceServers()');

        if (this._closed)
            throw new InvalidStateError('closed');
        else if (!Array.isArray(iceServers))
            throw new TypeError('missing iceServers');

        // Enqueue command.
        return this._awaitQueue.push(
            async () => this._handler.updateIceServers({ iceServers }));
    }

    /**
     * Produce a track.
     *
     * @param {MediaStreamTrack} track - Track to sent.
     * @param {Array<RTCRtpCodingParameters>} [encodings] - Encodings.
     * @param {Object} [codecOptions] - Codec options.
     * @param {Object} [appData={}] - Custom app data.
     *
     * @async
     * @returns {Producer}
     * @throws {InvalidStateError} if Transport closed or track ended.
     * @throws {TypeError} if wrong arguments.
     * @throws {UnsupportedError} if Transport direction is incompatible or
     *   cannot produce the given media kind.
     */
    async produce(
        {
            track,
            encodings,
            codecOptions,
            appData = {}
        } = {}
    )
    {
        console.log('produce() [track:%o]', track);

        if (!track)
            throw new TypeError('missing track');
        else if (this._direction !== 'send')
            throw new UnsupportedError('not a sending Transport');
        else if (!this._canProduceByKind[track.kind])
            throw new UnsupportedError(`cannot produce ${track.kind}`);
        else if (track.readyState === 'ended')
            throw new InvalidStateError('track ended');
        else if (appData && typeof appData !== 'object')
            throw new TypeError('if given, appData must be an object');

        // Enqueue command.
        return this._awaitQueue.push(
            async () =>
            {
                let normalizedEncodings;

                if (encodings && !Array.isArray(encodings))
                {
                    throw TypeError('encodings must be an array');
                }
                else if (encodings && encodings.length === 0)
                {
                    normalizedEncodings = undefined;
                }
                else if (encodings)
                {
                    normalizedEncodings = encodings
                        .map((encoding) =>
                        {
                            const normalizedEncoding = { active: true };

                            if (encoding.active === false)
                                normalizedEncoding.active = false;
                            if (typeof encoding.maxBitrate === 'number')
                                normalizedEncoding.maxBitrate = encoding.maxBitrate;
                            if (typeof encoding.maxFramerate === 'number')
                                normalizedEncoding.maxFramerate = encoding.maxFramerate;
                            if (typeof encoding.scaleResolutionDownBy === 'number')
                                normalizedEncoding.scaleResolutionDownBy = encoding.scaleResolutionDownBy;
                            if (typeof encoding.dtx === 'boolean')
                                normalizedEncoding.dtx = encoding.dtx;

                            return normalizedEncoding;
                        });
                }

                const { localId, rtpParameters } = await this._handler.send(
                    {
                        track,
                        encodings : normalizedEncodings,
                        codecOptions
                    });

                try
                {
                    const { id } = await this.safeEmitAsPromise(
                        'produce',
                        {
                            kind : track.kind,
                            rtpParameters,
                            appData
                        });

                    const producer =
                        new Producer({ id, localId, track, rtpParameters, appData });

                    this._producers.set(producer.id, producer);
                    this._handleProducer(producer);

                    return producer;
                }
                catch (error)
                {
                    this._handler.stopSending({ localId })
                        .catch(() => {});

                    throw error;
                }
            })
            // This catch is needed to stop the given track if the command above
            // failed due to closed Transport.
            .catch((error) =>
            {
                try { track.stop(); }
                catch (error2) {}

                throw error;
            });
    }

    /**
     * Consume a remote Producer.
     *
     * @param {String} id - Server-side Consumer id.
     * @param {String} producerId - Server-side Producer id.
     * @param {String} kind - 'audio' or 'video'.
     * @param {RTCRtpParameters} rtpParameters - Server-side Consumer RTP parameters.
     * @param {Object} [appData={}] - Custom app data.
     *
     * @async
     * @returns {Consumer}
     * @throws {InvalidStateError} if Transport closed.
     * @throws {TypeError} if wrong arguments.
     * @throws {UnsupportedError} if Transport direction is incompatible.
     */
    async consume(
        {
            id,
            producerId,
            kind,
            rtpParameters,
            appData = {}
        } = {})
    {
        console.log('consume()');

        if (this._closed)
            throw new InvalidStateError('closed');
        else if (this._direction !== 'recv')
            throw new UnsupportedError('not a receiving Transport');
        else if (typeof id !== 'string')
            throw new TypeError('missing id');
        else if (typeof producerId !== 'string')
            throw new TypeError('missing producerId');
        else if (kind !== 'audio' && kind !== 'video')
            throw new TypeError(`invalid kind "${kind}"`);
        else if (typeof rtpParameters !== 'object')
            throw new TypeError('missing rtpParameters');
        else if (appData && typeof appData !== 'object')
            throw new TypeError('if given, appData must be an object');

        // Enqueue command.
        return this._awaitQueue.push(
            async () =>
            {
                // Ensure the device can consume it.
                const canConsume = canReceive(
                    rtpParameters, this._extendedRtpCapabilities);

                if (!canConsume)
                    throw new UnsupportedError('cannot consume this Producer');

                const { localId, track } =
                    await this._handler.receive({ id, kind, rtpParameters });

                const consumer =
                    new Consumer({ id, localId, producerId, track, rtpParameters, appData });

                this._consumers.set(consumer.id, consumer);
                this._handleConsumer(consumer);

                return consumer;
            });
    }

    _handleHandler()
    {
        const handler = this._handler;

        handler.on('@connect', ({ dtlsParameters }, callback, errback) =>
        {
            if (this._closed)
            {
                errback(new InvalidStateError('closed'));

                return;
            }

            this.safeEmit('connect', { dtlsParameters }, callback, errback);
        });

        handler.on('@connectionstatechange', (connectionState) =>
        {
            if (connectionState === this._connectionState)
                return;

            console.log('connection state changed to %s', connectionState);

            this._connectionState = connectionState;

            if (!this._closed)
                this.safeEmit('connectionstatechange', connectionState);
        });
    }

    _handleProducer(producer)
    {
        producer.on('@close', () =>
        {
            this._producers.delete(producer.id);

            if (this._closed)
                return;

            this._awaitQueue.push(
                async () => this._handler.stopSending({ localId: producer.localId }))
                .catch((error) => console.log('producer.close() failed:%o', error));
        });

        producer.on('@replacetrack', (track, callback, errback) =>
        {
            this._awaitQueue.push(
                async () => this._handler.replaceTrack({ localId: producer.localId, track }))
                .then(callback)
                .catch(errback);
        });

        producer.on('@setmaxspatiallayer', (spatialLayer, callback, errback) =>
        {
            this._awaitQueue.push(
                async () => (
                    this._handler.setMaxSpatialLayer({ localId: producer.localId, spatialLayer })
                ))
                .then(callback)
                .catch(errback);
        });

        producer.on('@getstats', (callback, errback) =>
        {
            if (this._closed)
                return errback(new InvalidStateError('closed'));

            this._handler.getSenderStats({ localId: producer.localId })
                .then(callback)
                .catch(errback);
        });
    }

    _handleConsumer(consumer)
    {
        consumer.on('@close', () =>
        {
            this._consumers.delete(consumer.id);

            if (this._closed)
                return;

            this._awaitQueue.push(
                async () => this._handler.stopReceiving({ localId: consumer.localId }))
                .catch(() => {});
        });

        consumer.on('@getstats', (callback, errback) =>
        {
            if (this._closed)
                return errback(new InvalidStateError('closed'));

            this._handler.getReceiverStats({ localId: consumer.localId })
                .then(callback)
                .catch(errback);
        });
    }
}


/**********************************************************************************
handler.js

*************************************/

class utils{

static clone (obj)
{
    if (typeof obj !== 'object')
        return {};

    return JSON.parse(JSON.stringify(obj));
}

/**
 * Generates a random positive integer.
 *
 * @returns {Number}
 */
static generateRandomNumber()
{
    return Math.round(Math.random() * 10000000);
}

}

class Handler extends EnhancedEventEmitter
{
    constructor(
        {
            iceParameters,
            iceCandidates,
            dtlsParameters,
            iceServers,
            iceTransportPolicy,
            proprietaryConstraints
        }
    )
    {
        super();

        // Got transport local and remote parameters.
        // @type {Boolean}
        this._transportReady = false;

        // Remote SDP handler.
        // @type {RemoteSdp}
        this._remoteSdp = new RemoteSdp(
            {
                iceParameters,
                iceCandidates,
                dtlsParameters
            });


        console.log("arvind " + "RTCPeerConnection1" );

        // RTCPeerConnection instance.
        // @type {RTCPeerConnection}
        this._pc = new RTCPeerConnection(
            {
                iceServers         : iceServers || [],
                iceTransportPolicy : iceTransportPolicy || 'all',
                bundlePolicy       : 'max-bundle',
                rtcpMuxPolicy      : 'require',
                sdpSemantics       : 'unified-plan'
            },
            proprietaryConstraints);

        // Map of RTCTransceivers indexed by MID.
        // @type {Map<String, RTCTransceiver>}
        this._mapMidTransceiver = new Map();

        // Handle RTCPeerConnection connection status.
        this._pc.addEventListener('iceconnectionstatechange', () =>
        {
            switch (this._pc.iceConnectionState)
            {
                case 'checking':
                    this.emit('@connectionstatechange', 'connecting');
                    break;
                case 'connected':
                case 'completed':
                    this.emit('@connectionstatechange', 'connected');
                    break;
                case 'failed':
                    this.emit('@connectionstatechange', 'failed');
                    break;
                case 'disconnected':
                    this.emit('@connectionstatechange', 'disconnected');
                    break;
                case 'closed':
                    this.emit('@connectionstatechange', 'closed');
                    break;
            }
        });
    }

    close()
    {
        console.log('close()');

         console.log("arvind " + "_pc close close" );

        // Close RTCPeerConnection.
        try { this._pc.close(); }
        catch (error) {}
    }

    async getTransportStats()
    {
        return this._pc.getStats();
    }

    async updateIceServers({ iceServers })
    {

        console.log('updateIceServers()');

         console.log("arvind " + "_pc getConfiguration updateIceServers" );

        const configuration = this._pc.getConfiguration();

        configuration.iceServers = iceServers;

         console.log("arvind " + "_pc setConfiguration updateIceServers" );

        this._pc.setConfiguration(configuration);
    }

    async _setupTransport({ localDtlsRole, localSdpObject = null })
    {
        console.log("arvind " + "_pc localDescription _setupTransport" );

        if (!localSdpObject)
            localSdpObject = parse(this._pc.localDescription.sdp);

        // Get our local DTLS parameters.
        const dtlsParameters =
            extractDtlsParameters({ sdpObject: localSdpObject });

        // Set our DTLS role.
        dtlsParameters.role = localDtlsRole;

        // Update the remote DTLS role in the SDP.
        this._remoteSdp.updateDtlsRole(
            localDtlsRole === 'client' ? 'server' : 'client');

        // Need to tell the remote transport about our parameters.
        await this.safeEmitAsPromise('@connect', { dtlsParameters });

        this._transportReady = true;
    }
}

class SendHandler extends Handler
{
    constructor(data)
    {
        super(data);

        // Generic sending RTP parameters for audio and video.
        // @type {RTCRtpParameters}
        this._sendingRtpParametersByKind = data.sendingRtpParametersByKind;

        // Generic sending RTP parameters for audio and video suitable for the SDP
        // remote answer.
        // @type {RTCRtpParameters}
        this._sendingRemoteRtpParametersByKind = data.sendingRemoteRtpParametersByKind;

        // Local stream.
        // @type {MediaStream}
        this._stream = new MediaStream();
    }

    async send({ track, encodings, codecOptions })
    {
        console.log('send() [kind:%s, track.id:%s]', track.kind, track.id);

        if (encodings && encodings.length > 1)
        {
            encodings.forEach((encoding, idx) =>
            {
                encoding.rid = `r${idx}`;
            });
        }

         console.log("arvind " + "_pc track send" );

        const transceiver = this._pc.addTransceiver(
            track,
            {
                direction     : 'sendonly',
                streams       : [ this._stream ],
                sendEncodings : encodings
            });
         
         console.log("arvind " + "_pc createoffer send" );

        const offer = await this._pc.createOffer();
        let localSdpObject = parse(offer.sdp);
        const sendingRtpParameters =
            utils.clone(this._sendingRtpParametersByKind[track.kind]);

        if (!this._transportReady)
            await this._setupTransport({ localDtlsRole: 'server', localSdpObject });

   console.log(
      'send() | calling pc.setLocalDescription() [offer:%o]', offer);

        console.log(
            'send() | localSdpObject [offer:%o]', localSdpObject);

         console.log("arvind " + "_pc set localDescription send" );

        await this._pc.setLocalDescription(offer);

        // We can now get the transceiver.mid.
        const localId = transceiver.mid;

        // Set MID.
        sendingRtpParameters.mid = localId;

        localSdpObject = parse(this._pc.localDescription.sdp);

    console.log(
      'send() | localSdpObject after parse [offer:%o]', localSdpObject);

        const offerMediaObject = localSdpObject.media[localSdpObject.media.length - 1];

        console.log(
      'send() | offerMediaObject [offer:%o]', offerMediaObject);
        //console.log( JSON.stringify(offerMediaObject.candidates));

        // Set RTCP CNAME.
        sendingRtpParameters.rtcp.cname =
            getCname({ offerMediaObject });

        // Set RTP encodings by parsing the SDP offer if no encodings are given.
        if (!encodings)
        {
            sendingRtpParameters.encodings =
                sdpUnifiedPlanUtils.getRtpEncodings({ offerMediaObject });
        }
        // Set RTP encodings by parsing the SDP offer and complete them with given
        // one if just a single encoding has been given.
        else if (encodings.length === 1)
        {
            const newEncodings =
                sdpUnifiedPlanUtils.getRtpEncodings({ offerMediaObject });

            Object.assign(newEncodings[0], encodings[0]);

            sendingRtpParameters.encodings = newEncodings;
        }
        // Otherwise if more than 1 encoding are given use them verbatim.
        else
        {
            sendingRtpParameters.encodings = encodings;
        }

        // If VP8 and there is effective simulcast, add scalabilityMode to each
        // encoding.
        if (
            sendingRtpParameters.encodings.length > 1 &&
            sendingRtpParameters.codecs[0].mimeType.toLowerCase() === 'video/vp8'
        )
        {
            for (const encoding of sendingRtpParameters.encodings)
            {
                encoding.scalabilityMode = 'L1T3';
            }
        }

        this._remoteSdp.send(
            {
                offerMediaObject,
                offerRtpParameters  : sendingRtpParameters,
                answerRtpParameters : this._sendingRemoteRtpParametersByKind[track.kind],
                codecOptions
            });




    console.log(
      'send() | sendingRtpParameters [offer:%o]', sendingRtpParameters);

     console.log(
      'send() | answerRtpParameters [offer:%o]', this._sendingRemoteRtpParametersByKind[track.kind]);

        const answer = { type: 'answer', sdp: this._remoteSdp.getSdp() };

        console.log(
            'send() | calling pc.setRemoteDescription() [answer:%o]', answer);

         console.log("arvind " + "_pc setRemoteDescription send" );

        await this._pc.setRemoteDescription(answer);

        // Store in the map.
        this._mapMidTransceiver.set(localId, transceiver);

        return { localId, rtpParameters: sendingRtpParameters };
    }

    async stopSending({ localId })
    {
        console.log('stopSending() [localId:%s]', localId);

        const transceiver = this._mapMidTransceiver.get(localId);

        if (!transceiver)
            throw new Error('associated RTCRtpTransceiver not found');

         console.log("arvind " + "_pc removeTrack stopSending" );

        transceiver.sender.replaceTrack(null);
        this._pc.removeTrack(transceiver.sender);
        this._remoteSdp.disableMediaSection(transceiver.mid);


         console.log("arvind " + "create offer stopsending" );

        const offer = await this._pc.createOffer();

        console.log(
            'stopSending() | calling pc.setLocalDescription() [offer:%o]', offer);

         console.log("arvind " + "_pc setLocalDescription stop sending" );

        await this._pc.setLocalDescription(offer);

        const answer = { type: 'answer', sdp: this._remoteSdp.getSdp() };

        console.log(
            'stopSending() | calling pc.setRemoteDescription() [answer:%o]', answer);

         console.log("arvind " + "_pc setRemoteDescription stopsending" );

        await this._pc.setRemoteDescription(answer);
    }

    async replaceTrack({ localId, track })
    {
        console.log('replaceTrack() [localId:%s, track.id:%s]', localId, track.id);

        const transceiver = this._mapMidTransceiver.get(localId);

        if (!transceiver)
            throw new Error('associated RTCRtpTransceiver not found');

        await transceiver.sender.replaceTrack(track);
    }

    async setMaxSpatialLayer({ localId, spatialLayer })
    {
        console.log(
            'setMaxSpatialLayer() [localId:%s, spatialLayer:%s]',
            localId, spatialLayer);

        const transceiver = this._mapMidTransceiver.get(localId);

        if (!transceiver)
            throw new Error('associated RTCRtpTransceiver not found');

        const parameters = transceiver.sender.getParameters();

        parameters.encodings.forEach((encoding, idx) =>
        {
            if (idx <= spatialLayer)
                encoding.active = true;
            else
                encoding.active = false;
        });

        await transceiver.sender.setParameters(parameters);
    }

    async getSenderStats({ localId })
    {
        const transceiver = this._mapMidTransceiver.get(localId);

        if (!transceiver)
            throw new Error('associated RTCRtpTransceiver not found');

        return transceiver.sender.getStats();
    }

    async restartIce({ iceParameters })
    {
        console.log('restartIce()');

        // Provide the remote SDP handler with new remote ICE parameters.
        this._remoteSdp.updateIceParameters(iceParameters);

        if (!this._transportReady)
            return;

         console.log("arvind " + "_pc createoffer for restart ice" );

        const offer = await this._pc.createOffer({ iceRestart: true });

        console.log(
            'restartIce() | calling pc.setLocalDescription() [offer:%o]', offer);


         console.log("arvind " + "_pc setLocalDescription restart ice" );

        await this._pc.setLocalDescription(offer);

        const answer = { type: 'answer', sdp: this._remoteSdp.getSdp() };

        console.log(
            'restartIce() | calling pc.setRemoteDescription() [answer:%o]', answer);

         console.log("arvind " + "_pc setRemoteDescription restart ice" );

        await this._pc.setRemoteDescription(answer);
    }
}

class RecvHandler extends Handler
{
    constructor(data)
    {
        super(data);

        // MID value counter. It must be converted to string and incremented for
        // each new m= section.
        // @type {Number}
        this._nextMid = 0;
    }

    async receive({ id, kind, rtpParameters })
    {
        console.log('receive() [id:%s, kind:%s]', id, kind);

        const localId = String(this._nextMid);

        this._remoteSdp.receive(
            {
                mid                : localId,
                kind,
                offerRtpParameters : rtpParameters,
                streamId           : rtpParameters.rtcp.cname,
                trackId            : id
            });

        const offer = { type: 'offer', sdp: this._remoteSdp.getSdp() };

        console.log(
            'receive() | calling pc.setRemoteDescription() [offer:%o]', offer);

         console.log("arvind " + "_pc setRemoteDescription receive" );

        await this._pc.setRemoteDescription(offer);

        let answer = await this._pc.createAnswer();
        const localSdpObject = parse(answer.sdp);
        const answerMediaObject = localSdpObject.media
            .find((m) => String(m.mid) === localId);

        // May need to modify codec parameters in the answer based on codec
        // parameters in the offer.
        applyCodecParameters(
            {
                offerRtpParameters : rtpParameters,
                answerMediaObject
            });

        answer = { type: 'answer', sdp: write1(localSdpObject) };

        if (!this._transportReady)
            await this._setupTransport({ localDtlsRole: 'client', localSdpObject });

        console.log(
            'receive() | calling pc.setLocalDescription() [answer:%o]', answer);

         console.log("arvind " + "_pc setLocalDescription receive" );

        await this._pc.setLocalDescription(answer);

         console.log("arvind " + "_pc getTransceivers receive" );

        const transceiver = this._pc.getTransceivers()
            .find((t) => t.mid === localId);

        if (!transceiver)
            throw new Error('new RTCRtpTransceiver not found');

        // Store in the map.
        this._mapMidTransceiver.set(localId, transceiver);

        // Increase next MID.
        this._nextMid++;

        return { localId, track: transceiver.receiver.track };
    }

    async stopReceiving({ localId })
    {
        console.log('stopReceiving() [localId:%s]', localId);

        const transceiver = this._mapMidTransceiver.get(localId);

        if (!transceiver)
            throw new Error('associated RTCRtpTransceiver not found');

        this._remoteSdp.disableMediaSection(transceiver.mid);

        const offer = { type: 'offer', sdp: this._remoteSdp.getSdp() };

        console.log(
            'stopReceiving() | calling pc.setRemoteDescription() [offer:%o]', offer);


         console.log("arvind " + "_pc setRemoteDescription stop receive" );


        await this._pc.setRemoteDescription(offer);

         console.log("arvind " + "_pc createAnswer stop receive" );

        const answer = await this._pc.createAnswer();

        console.log(
            'stopReceiving() | calling pc.setLocalDescription() [answer:%o]', answer);


         console.log("arvind " + "_pc localDescription stop receive" );

        await this._pc.setLocalDescription(answer);
    }

    async getReceiverStats({ localId })
    {
        const transceiver = this._mapMidTransceiver.get(localId);

        if (!transceiver)
            throw new Error('associated RTCRtpTransceiver not found');

        return transceiver.receiver.getStats();
    }

    async restartIce({ iceParameters })
    {
        console.log('restartIce()');

        // Provide the remote SDP handler with new remote ICE parameters.
        this._remoteSdp.updateIceParameters(iceParameters);

        if (!this._transportReady)
            return;

        const offer = { type: 'offer', sdp: this._remoteSdp.getSdp() };

        console.log(
            'restartIce() | calling pc.setRemoteDescription() [offer:%o]', offer);

         console.log("arvind " + "_pc setRemoteDescription restart ICE" );

        await this._pc.setRemoteDescription(offer);


         console.log("arvind " + "_pc createAnswer restart ice" );
        const answer = await this._pc.createAnswer();

        console.log(
            'restartIce() | calling pc.setLocalDescription() [answer:%o]', answer);

         console.log("arvind " + "_pc setLocalDescription restart ice" );

        await this._pc.setLocalDescription(answer);
    }
}



/**********************************************************************
Producer 

***********************************************************************/
class Producer extends EnhancedEventEmitter
{
    /**
     * @private
     *
     * @emits transportclose
     * @emits trackended
     * @emits {track: MediaStreamTrack} @replacetrack
     * @emits {spatialLayer: String} @setmaxspatiallayer
     * @emits @getstats
     * @emits @close
     */
    constructor({ id, localId, track, rtpParameters, appData })
    {
        super();

        // Id.
        // @type {String}
        this._id = id;

        // Local id.
        // @type {String}
        this._localId = localId;

        // Closed flag.
        // @type {Boolean}
        this._closed = false;

        // Local track.
        // @type {MediaStreamTrack}
        this._track = track;

        // RTP parameters.
        // @type {RTCRtpParameters}
        this._rtpParameters = rtpParameters;

        // Paused flag.
        // @type {Boolean}
        this._paused = !track.enabled;

        // Video max spatial layer.
        // @type {Number|Undefined}
        this._maxSpatialLayer = undefined;

        // App custom data.
        // @type {Object}
        this._appData = appData;

        this._onTrackEnded = this._onTrackEnded.bind(this);

        this._handleTrack();
    }

    /**
     * Producer id.
     *
     * @returns {String}
     */
    get id()
    {
        return this._id;
    }

    /**
     * Local id.
     *
     * @private
     * @returns {String}
     */
    get localId()
    {
        return this._localId;
    }

    /**
     * Whether the Producer is closed.
     *
     * @returns {Boolean}
     */
    get closed()
    {
        return this._closed;
    }

    /**
     * Media kind.
     *
     * @returns {String}
     */
    get kind()
    {
        return this._track.kind;
    }

    /**
     * The associated track.
     *
     * @returns {MediaStreamTrack}
     */
    get track()
    {
        return this._track;
    }

    /**
     * RTP parameters.
     *
     * @returns {RTCRtpParameters}
     */
    get rtpParameters()
    {
        return this._rtpParameters;
    }

    /**
     * Whether the Producer is paused.
     *
     * @returns {Boolean}
     */
    get paused()
    {
        return this._paused;
    }

    /**
     * Max spatial layer.
     *
     * @type {Number}
     */
    get maxSpatialLayer()
    {
        return this._maxSpatialLayer;
    }

    /**
     * App custom data.
     *
     * @returns {Object}
     */
    get appData()
    {
        return this._appData;
    }

    /**
     * Invalid setter.
     */
    set appData(appData) // eslint-disable-line no-unused-vars
    {
        throw new Error('cannot override appData object');
    }

    /**
     * Closes the Producer.
     */
    close()
    {
        if (this._closed)
            return;

        console.log('close()');

        this._closed = true;

        this._destroyTrack();

        this.emit('@close');
    }

    /**
     * Transport was closed.
     *
     * @private
     */
    transportClosed()
    {
        if (this._closed)
            return;

        console.log('transportClosed()');

        this._closed = true;

        this._destroyTrack();

        this.safeEmit('transportclose');
    }

    /**
     * Get associated RTCRtpSender stats.
     *
     * @promise
     * @returns {RTCStatsReport}
     * @throws {InvalidStateError} if Producer closed.
     */
    async getStats()
    {
        if (this._closed)
            throw new InvalidStateError('closed');

        return this.safeEmitAsPromise('@getstats');
    }

    /**
     * Pauses sending media.
     */
    pause()
    {
        console.log('pause()');

        if (this._closed)
        {
            console.error('pause() | Producer closed');

            return;
        }

        this._paused = true;
        this._track.enabled = false;
    }

    /**
     * Resumes sending media.
     */
    resume()
    {
        console.log('resume()');

        if (this._closed)
        {
            console.error('resume() | Producer closed');

            return;
        }

        this._paused = false;
        this._track.enabled = true;
    }

    /**
     * Replaces the current track with a new one.
     *
     * @param {MediaStreamTrack} track - New track.
     *
     * @async
     * @throws {InvalidStateError} if Producer closed or track ended.
     * @throws {TypeError} if wrong arguments.
     */
    async replaceTrack({ track } = {})
    {
        console.log('replaceTrack() [track:%o]', track);

        if (this._closed)
        {
            // This must be done here. Otherwise there is no chance to stop the given
            // track.
            try { track.stop(); }
            catch (error) {}

            throw new InvalidStateError('closed');
        }
        else if (!track)
        {
            throw new TypeError('missing track');
        }
        else if (track.readyState === 'ended')
        {
            throw new InvalidStateError('track ended');
        }

        await this.safeEmitAsPromise('@replacetrack', track);

        // Destroy the previous track.
        this._destroyTrack();

        // Set the new track.
        this._track = track;

        // If this Producer was paused/resumed and the state of the new
        // track does not match, fix it.
        if (!this._paused)
            this._track.enabled = true;
        else
            this._track.enabled = false;

        // Handle the effective track.
        this._handleTrack();
    }

    /**
     * Sets the video max spatial layer to be sent.
     *
     * @param {Number} spatialLayer
     *
     * @async
     * @throws {InvalidStateError} if Producer closed.
     * @throws {UnsupportedError} if not a video Producer.
     * @throws {TypeError} if wrong arguments.
     */
    async setMaxSpatialLayer(spatialLayer)
    {
        if (this._closed)
            throw new InvalidStateError('closed');
        else if (this._track.kind !== 'video')
            throw new UnsupportedError('not a video Producer');
        else if (typeof spatialLayer !== 'number')
            throw new TypeError('invalid spatialLayer');

        if (spatialLayer === this._maxSpatialLayer)
            return;

        await this.safeEmitAsPromise('@setmaxspatiallayer', spatialLayer);

        this._maxSpatialLayer = spatialLayer;
    }

    /**
     * @private
     */
    _onTrackEnded()
    {
        console.log('track "ended" event');

        this.safeEmit('trackended');
    }

    /**
     * @private
     */
    _handleTrack()
    {
        this._track.addEventListener('ended', this._onTrackEnded);
    }

    /**
     * @private
     */
    _destroyTrack()
    {
        try
        {
            this._track.removeEventListener('ended', this._onTrackEnded);
            this._track.stop();
        }
        catch (error)
        {}
    }
}

/***********************************************************************************

consumer.js

**********************************************************************************/

class Consumer extends EnhancedEventEmitter
{
    /**
     * @private
     *
     * @emits transportclose
     * @emits trackended
     * @emits @getstats
     * @emits @close
     */
    constructor({ id, localId, producerId, track, rtpParameters, appData })
    {
        super();

        // Id.
        // @type {String}
        this._id = id;

        // Local id.
        // @type {String}
        this._localId = localId;

        // Associated Producer id.
        // @type {String}
        this._producerId = producerId;

        // Closed flag.
        // @type {Boolean}
        this._closed = false;

        // Remote track.
        // @type {MediaStreamTrack}
        this._track = track;

        // RTP parameters.
        // @type {RTCRtpParameters}
        this._rtpParameters = rtpParameters;

        // Paused flag.
        // @type {Boolean}
        this._paused = !track.enabled;

        // App custom data.
        // @type {Object}
        this._appData = appData;

        this._onTrackEnded = this._onTrackEnded.bind(this);

        this._handleTrack();
    }

    /**
     * Consumer id.
     *
     * @returns {String}
     */
    get id()
    {
        return this._id;
    }

    /**
     * Local id.
     *
     * @private
     * @returns {String}
     */
    get localId()
    {
        return this._localId;
    }

    /**
     * Associated Producer id.
     *
     * @returns {String}
     */
    get producerId()
    {
        return this._producerId;
    }

    /**
     * Whether the Consumer is closed.
     *
     * @returns {Boolean}
     */
    get closed()
    {
        return this._closed;
    }

    /**
     * Media kind.
     *
     * @returns {String}
     */
    get kind()
    {
        return this._track.kind;
    }

    /**
     * The associated track.
     *
     * @returns {MediaStreamTrack}
     */
    get track()
    {
        return this._track;
    }

    /**
     * RTP parameters.
     *
     * @returns {RTCRtpParameters}
     */
    get rtpParameters()
    {
        return this._rtpParameters;
    }

    /**
     * Whether the Consumer is paused.
     *
     * @returns {Boolean}
     */
    get paused()
    {
        return this._paused;
    }

    /**
     * App custom data.
     *
     * @returns {Object}
     */
    get appData()
    {
        return this._appData;
    }

    /**
     * Invalid setter.
     */
    set appData(appData) // eslint-disable-line no-unused-vars
    {
        throw new Error('cannot override appData object');
    }

    /**
     * Closes the Consumer.
     */
    close()
    {
        if (this._closed)
            return;

        console.log('close()');

        this._closed = true;

        this._destroyTrack();

        this.emit('@close');
    }

    /**
     * Transport was closed.
     *
     * @private
     */
    transportClosed()
    {
        if (this._closed)
            return;

        console.log('transportClosed()');

        this._closed = true;

        this._destroyTrack();

        this.safeEmit('transportclose');
    }

    /**
     * Get associated RTCRtpReceiver stats.
     *
     * @async
     * @returns {RTCStatsReport}
     * @throws {InvalidStateError} if Consumer closed.
     */
    async getStats()
    {
        if (this._closed)
            throw new InvalidStateError('closed');

        return this.safeEmitAsPromise('@getstats');
    }

    /**
     * Pauses receiving media.
     */
    pause()
    {
        console.log('pause()');

        if (this._closed)
        {
            console.error('pause() | Consumer closed');

            return;
        }

        this._paused = true;
        this._track.enabled = false;
    }

    /**
     * Resumes receiving media.
     */
    resume()
    {
        console.log('resume()');

        if (this._closed)
        {
            console.error('resume() | Consumer closed');

            return;
        }

        this._paused = false;
        this._track.enabled = true;
    }

    /**
     * @private
     */
    _onTrackEnded()
    {
        console.log('track "ended" event');

        this.safeEmit('trackended');
    }

    /**
     * @private
     */
    _handleTrack()
    {
        this._track.addEventListener('ended', this._onTrackEnded);
    }

    /**
     * @private
     */
    _destroyTrack()
    {
        try
        {
            this._track.removeEventListener('ended', this._onTrackEnded);
            this._track.stop();
        }
        catch (error)
        {}
    }
}

