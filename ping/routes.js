module.exports = function(app,express){
	app.use(express.static('./public'));
    app.use(express.static('./javascript'));
    app.use(express.static('./files'));
    app.use(express.static('./images'));
    app.use(express.static('./views'));
    app.use(express.static('./stylesheets'));
	app.get('/',function(req,res){
		//res.render('index');
		res.sendFile('/views/ping.html');
	});
	app.get('/ping',function(req,res){
		res.send('show ping');
	});
	app.get('/gdiPing',function(req,res){
		res.send('show gdiPing');
	});
};