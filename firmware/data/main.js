function initSettings() {
    document.getElementById("reset-settings").href = "javascript:void(0)"
}

function reset() {
    var result = confirm("Would you like to reset all settings and WiFi credentials?")

    if (result) {
        window.location.replace("/reset")
    }
    else {
        window.location.replace("/settings")
    }
}
