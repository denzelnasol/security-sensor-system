function addListItem(fileName) {
    var videoLink = $('<a>').attr('href', `/recordings/${fileName}`).text(fileName);
    var listItem = $('<li>').append(videoLink);
    listItem.addClass('list-group-item')

    // add an event listener to each list item that opens the video in a new window/tab when clicked
    listItem.on('click', function (e) {
        e.preventDefault();
        window.open($(this).find('a').attr('href'), '_blank');
    });

    $('#video-list').append(listItem);
}

$(document).ready(function () {
    $.ajax({
        url: '/recordings',
        success: function (files) {
            for (let i = 0; i < files.length; i++) {
                addListItem(files[i]);
            }
        },
        error: function (jqXHR, textStatus, errorThrown) {
            console.log('Error retrieving file list: ' + textStatus + ', ' + errorThrown);
        }
    });
});