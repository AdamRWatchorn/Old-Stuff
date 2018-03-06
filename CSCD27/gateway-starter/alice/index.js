var request = require('request');

var config = {};

var do_request = function(){
    request.get({url: "http://welcome.seclab.space/", timeout: 2000}, function (err, response, body) {
        if (err) return console.log(err);
        if (response.statusCode != 200) return console.log(response.statusCode);
        return console.log(body);
    });
};

setInterval(do_request, 1 * 3000);