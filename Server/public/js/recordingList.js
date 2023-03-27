$(document).ready(function () {
    $.ajax({
        url: '/recordings',
        success: function (files) {
            for (let i = 0; i < files.length; i++) {
                const fileName = files[i];
                const videoLink = $('<a>').attr('href', `/recordings/${fileName}`).text(fileName);
                const listItem = $('<li>').append(videoLink);

                // add an event listener to each list item that opens the video in a new window/tab when clicked
                listItem.on('click', function (e) {
                    e.preventDefault();
                    window.open($(this).find('a').attr('href'), '_blank');
                });

                $('#video-list').append(listItem);
            }
        },
        error: function (jqXHR, textStatus, errorThrown) {
            console.log('Error retrieving file list: ' + textStatus + ', ' + errorThrown);
        }
    });
});