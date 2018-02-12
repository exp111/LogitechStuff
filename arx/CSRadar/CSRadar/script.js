onPropertyUpdate = function ()
{
    var c = document.getElementById('myCanvas');
    var ctx = c.getContext('2d');

    ctx.clearRect(0, 0, 500, 500);

    var json = document.getElementById('json').innerHTML;
    var jsonParsed = JSON.parse(json);

    if (jsonParsed.hasOwnProperty('data')) {
        for (i = 0; i < jsonParsed.data.length; i++) {
            ctx.fillStyle = jsonParsed.data[i].enemy ? 'red' : 'green';
            ctx.font = '20px Comic Sans MS';
            ctx.fillText(jsonParsed.data[i].name, 250 + jsonParsed.data[i].x, 250 - jsonParsed.data[i].y);
            ctx.fillRect(250 + jsonParsed.data[i].x, 250 - jsonParsed.data[i].y, 25, 25);
        }
    }

    ctx.moveTo(250, 0);
    ctx.lineTo(250, 500);
    ctx.stroke();
    ctx.moveTo(0, 250);
    ctx.lineTo(500, 250);
    ctx.stroke();
}