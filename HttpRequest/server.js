var express = require('express');
var app = express();

// parsing body
app.use(express.json())
app.use(express.urlencoded({extended:false}))

app.post('/level', async(req,res)=>{
    console.log("level");
    console.log(req.body);
    console.log(req.params);

    res.sendStatus(201);
})


