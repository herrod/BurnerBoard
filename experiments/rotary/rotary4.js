
var exec = require('child_process').exec;

console.log('starting');
var bone = require('/usr/local/lib/node_modules/bonescript');
console.log('after require');

// Hack to make sure gpio 23,47 are available
cmd = exec("echo 47 >/sys/class/gpio/unexport");
cmd = exec("echo 23 >/sys/class/gpio/unexport");

bone.pinMode('P8_13', bone.INPUT, 7, 'pullup', 'fast');
bone.pinMode('P8_15', bone.INPUT, 7, 'pullup', 'fast');

//setInterval(check, 10);
bone.attachInterrupt('P8_13', check, bone.FALLING);
bone.attachInterrupt('P8_15', check, bone.FALLING);

setInterval(heartbeat, 1000);

var encoder_pos = 0;

var last_check_timestamp;

var a;
var b;
function check(x) {
  var enc;
  var hrtime = process.hrtime();
  var now = hrtime[1] / 1000 + hrtime[0] * 1000000; 

  if ((now - last_check_timestamp) > 2000000)
    console.log(''); 

  console.log('delay = ' + (now - last_check_timestamp));

  if ((now - last_check_timestamp) < 1000) {
        last_check_timestamp = now;
//	return;
  }
  last_check_timestamp = now;

//  console.log(JSON.stringify(x));
  if (x.pin.key == "P8_13") {
    b = x.value;
//    console.log("b = " + b);
  }
  if (x.pin.key == "P8_15") {
    a = x.value;
//    console.log("a = " + a);
  }

//  bone.digitalRead('P8_13', printStatus);
//  bone.digitalRead('P8_15', printStatus);
  enc = read_encoder(a, b);
//  if (enc)
//    console.log('enc = ' + enc);
  if (enc == 1) {
    encoder_pos++;
    console.log('    encoder_pos = ' + encoder_pos);
  }
  if (enc == 3) {
    encoder_pos--
    console.log('    encoder_pos = ' + encoder_pos);
  }
}

var old_state = 0;

// returns change in encoder state (-1,0,1)
function read_encoder(a, b)
{
  var new_state = 0;
  var return_state;
  new_state = (b * 4) | (a * 2) | (a ^ b);
  return_state = (new_state - old_state) % 4;
  console.log('  a:' + a + ' b: ' + b + ' new_state = ' + new_state + ' return = ' + return_state);
  old_state = new_state;
  return (return_state);
}

function printStatus(x) {
    console.log('x.value = ' + x.value);
    console.log('x.err = ' + x.err);
}



function heartbeat() {
}
