
var exec = require('child_process').exec;


console.log('starting');
var b = require('/usr/local/lib/node_modules/bonescript');
console.log('after require');

// Hack to make sure gpio 23,47 are available
cmd = exec("echo 47 >/sys/class/gpio/unexport");
cmd = exec("echo 23 >/sys/class/gpio/unexport");

b.pinMode('P8_13', b.INPUT_PULLUP);
b.pinMode('P8_15', b.INPUT_PULLUP);

setInterval(check, 1000);

//b.attachInterrupt('P8_15', true, b.RISING, interruptCallback);
//setTimeout(detach, 120000);

//function interruptCallback(x) {
//    console.log(JSON.stringify(x));
//}

//function detach() {
//    b.detachInterrupt('P8_15');
//    console.log('Interrupt detached');
//}

function check() {
  b.digitalRead('P8_13', printStatus);
  b.digitalRead('P8_15', printStatus);
}

function printStatus(x) {
    console.log('x.value = ' + x.value);
    console.log('x.err = ' + x.err);
}




