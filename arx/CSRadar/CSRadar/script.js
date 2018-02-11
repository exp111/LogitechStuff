function check() {
    //get canvas
    var c = document.getElementById('myCanvas');
    var ctx = c.getContext('2d');

    //clear
    ctx.clearRect(0, 0, 500, 500);

    //Json parse
    var json = document.getElementById('json').innerHTML;
    var jsonParsed = JSON.parse(json);

    if (jsonParsed.hasOwnProperty('data')) {
        //draw the players
        for (i = 0; i < jsonParsed.data.length; i++) {
            ctx.fillStyle = jsonParsed.data[i].enemy ? 'red' : 'green';
            ctx.font = '20px Comic Sans MS';
            ctx.fillText(jsonParsed.data[i].name, 250 + jsonParsed.data[i].x, 250 + jsonParsed.data[i].y);
            ctx.fillRect(250 + jsonParsed.data[i].x, 250 + jsonParsed.data[i].y, 25, 25);
        }
    }

    //Radar indicator
    ctx.moveTo(250, 0); //top mid
    ctx.lineTo(250, 500); //down mid
    ctx.stroke();
    ctx.moveTo(0, 250); //left mid
    ctx.lineTo(500, 250); //right mid
    ctx.stroke();
}

setInterval(function () { check(); }, 100);