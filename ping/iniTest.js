var ini = require('node-ini');
ini.parse('./config.ini', function(err,data){
  if(err) console.log(err)
  else {
    console.log(data)
  }
});