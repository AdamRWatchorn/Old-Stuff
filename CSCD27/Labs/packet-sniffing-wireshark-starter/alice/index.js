var request = require('request');

var config = {};

config.http = {
    host: "http-only.seclab.space",
    data: {
        username: "alice",
        password: "pass4alice"
    }
};

config.https = {
    host: "https-only.seclab.space",
    data:{
        username: "alice",
        password: "h4x00r"
    }
};

var protocol = 'http';

var do_request = function(){
    protocol = (protocol === 'http')? 'https' : 'http';
    request.post(protocol + "://"+ config[protocol].host + "/", {form:config[protocol].data}, function (err, response, body) {
        if (err) return console.log(err);
        if (response.statusCode != 200) return console.log(response.statusCode);
        return console.log(body);
    });
};

setInterval(do_request, 1 * 1000);