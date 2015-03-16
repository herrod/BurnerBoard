
var exec = require('child_process').exec;


console.log('starting');
var bone = require('/usr/local/lib/node_modules/bonescript');
console.log('after require');

// Hack to make sure gpio 23,47 are available
cmd = exec("echo 47 >/sys/class/gpio/unexport");
cmd = exec("echo 23 >/sys/class/gpio/unexport");

bone.pinMode('P8_13', bone.INPUT, 7, 'pullup', 'fast', printStatus);
bone.pinMode('P8_15', bone.INPUT, 7, 'pullup', 'fast', printStatus);

//setInterval(check, 10);
bone.attachInterrupt('P8_13', check, bone.FALLING);
bone.attachInterrupt('P8_15', check, bone.FALLING);


setInterval(heartbeat, 1000);

function check(x) {
  var enc;

  console.log(JSON.stringify(x));


//  bone.digitalRead('P8_13', printStatus);
//  bone.digitalRead('P8_15', printStatus);
  enc = read_encoder();
//  if (enc)
    console.log('enc = ' + enc);
}


// returns change in encoder state (-1,0,1)
var old_AB = 0;
function read_encoder()
{
  var enc_states = [0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0];
  var a, b;
  /**/
  old_AB <<= 2;                   //remember previous state
  old_AB &= 0xff;
//  old_AB |= ( ENC_PORT & 0x03 );  //add current state
  a = b = 0;
  if (bone.digitalRead('P8_13')) 
    a = 1;
  if (bone.digitalRead('P8_15'))
    b = 1;
  old_AB |= a | (b <<1);
  return ( enc_states[( old_AB & 0x0f )]);
}

function printStatus(x) {
    console.log('x.value = ' + x.value);
    console.log('x.err = ' + x.err);
}



function heartbeat() {
}
