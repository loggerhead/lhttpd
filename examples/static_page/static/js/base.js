function resizeImg() {
    var img = document.getElementById("img");
    var maxWidth = document.body.clientWidth;
    var maxHeight = document.body.clientHeight;

    var w = img.width;
    var h = img.height;

    h *= maxWidth / w;
    w = maxWidth;
    if (h > maxHeight) {
        w *= maxHeight / h;
        h = maxHeight;
    }

    img.width = w;
    img.height = h;
}