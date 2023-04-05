var express = require('express');
var app = express();

var port = 8080;

// parsing body
app.use(express.json())
app.use(express.urlencoded({extended:false}))

app.post('/level', async(req,res)=>{
    console.log("level");
    console.log(req.body);
	console.log(req.body.name);
    res.sendStatus(201);
})

app.listen(port, function(){
    console.log(`app running on port ${port}`)
})

