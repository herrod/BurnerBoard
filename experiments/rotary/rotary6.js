
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
bone.attachInterrupt('P8_13', check, bone.CHANGE);
bone.attachInterrupt('P8_15', check, bone.CHANGE);

//setInterval(check, 2);

var encoder_pos = 0;

var last_check_timestamp;

function check(x) {
  var enc;
  var hrtime = process.hrtime();
  var now = hrtime[1] / 1000 + hrtime[0] * 1000000; 

//  if ((now - last_check_timestamp) > 2000000)
//    console.log(''); 

//  console.log('delay = ' + (now - last_check_timestamp));

  if ((now - last_check_timestamp) < 1000) {
        last_check_timestamp = now;
//	return;
  }
  last_check_timestamp = now;

  enc = read_encoder();
  if (enc) {
    console.log('enc = ' + enc);
    encoder_pos += enc;
    console.log('    encoder_pos = ' + encoder_pos);
  }
}

var old_ab = 0;

// returns change in encoder state (-1,0,1)
function read_encoder(a, b)
{
//  var enc_states = [0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0];
  var enc_states = [0,-1,1,0,0,0,0,0,0,0,0,0,0,0,0,0];
  var a;
  var b;
  a = bone.digitalRead('P8_13');
  b = bone.digitalRead('P8_15');


  old_ab <<= 2;                   //remember previous state
  old_ab &= 0x0f;
  old_ab |= a | (b <<1);
  if (enc_states[old_ab])
    console.log('  a:' + a + ' b: ' + b + ' oldab = ' + old_ab + ' enc_states = ' + enc_states[old_ab]);
  return (enc_states[old_ab]);
}

function printStatus(x) {
    console.log('x.value = ' + x.value);
    console.log('x.err = ' + x.err);
}



function heartbeat() {
}




