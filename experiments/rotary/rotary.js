
var exec = require('child_process').exec;


console.log('starting');
var bone = require('/usr/local/lib/node_modules/bonescript');
console.log('after require');

// Hack to make sure gpio 23,47 are available
cmd = exec("echo 47 >/sys/class/gpio/unexport");
cmd = exec("echo 23 >/sys/class/gpio/unexport");

bone.pinMode('P8_13', bone.INPUT_PULLUP);
bone.pinMode('P8_15', bone.INPUT_PULLUP);

setInterval(check, 10);

function check() {
  var enc;

//  bone.digitalRead('P8_13', printStatus);
//  bone.digitalRead('P8_15', printStatus);
  enc = read_encoder();
  if (enc)
    console.log('enc = ' + enc);
}


// returns change in encoder state (-1,0,1)
function read_encoder()
{
  var enc_states = [0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0];
  var old_AB = 0;
  var a, b;
  /**/
  old_AB <<= 2;                   //remember previous state
//  old_AB |= ( ENC_PORT & 0x03 );  //add current state
  a = bone.digitalRead('P8_13');
  b = bone.digitalRead('P8_15');
  old_AB |= a | (b <<1);
  return ( enc_states[( old_AB & 0x0f )]);
}

function printStatus(x) {
    console.log('x.value = ' + x.value);
    console.log('x.err = ' + x.err);
}




