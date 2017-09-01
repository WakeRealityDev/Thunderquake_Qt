/*
  Client-side code for Remote-IF demo (AJAX version).

  Written by Andrew Plotkin. This script is in the public domain.
 */

/*
  Data back to C++ land?
  https://stackoverflow.com/questions/45330481/qtwebengine-synchronously-execute-javascript-to-read-function-result
  https://stackoverflow.com/questions/4975658/qt-qwebview-javascript-callback
  Ok, the proper Qt5 way:
  https://stackoverflow.com/questions/27512526/expose-c-object-to-javascript-in-qt-with-qtwebengine
  */

var jshelper;
var backToApp = undefined;

function accept(arg) {
    console.log("need to get JSON now: " + JSON.stringify(arg));

    if (backToApp === undefined) {
        backToApp = new QWebChannel(qt.webChannelTransport, function(channel) {
            jshelper = channel.objects.qtRemGlkForwarder;
            jshelper.toRemGlkEngine(JSON.stringify(arg));
        });
    } else {
        jshelper.toRemGlkEngine(JSON.stringify(arg));
    };

    // qtRemGlkForwarder.toRemGlkEngine(JSON.stringify(arg));
};

function callback_error(jqxhr, status, error) {
    var msg = status;
    if (error)
        msg = msg + ': ' + error;
    GlkOte.error('Server error: ' + msg);
};

function callback_success(data, status, jqxhr) {
    GlkOte.update(data);
}

function incomingA(inPayload) {
    console.log(":::::::::::::::: incomingA ::::::::::");
    console.log(inPayload);
    // make into JSON object
    var asJSON = JSON.parse(inPayload);
    GlkOte.update(asJSON);
}

Game = {
    accept: accept,
};

/* The page-ready handler. Like onload(), but better, I'm told. */
$(document).ready(function() {
    if (use_gidebug) {
        Game.debug_commands = true;
        Game.debug_console_open = true;
    }
    GlkOte.init();
    console.log("after GlkOte.init()");
});


