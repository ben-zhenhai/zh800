module.exports = function(app){
	app.get('/',function(req,res){
		res.render('index');
	});
	app.get('/ping',function(req,res){
		res.send('show ping');
	});
	app.get('/gdiPing',function(req,res){
		res.send('show gdiPing');
	});
};