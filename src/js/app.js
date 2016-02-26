var mConfig = {};

Pebble.addEventListener("ready", function(e) {
    loadLocalData();
    //console.log("Local data loaded: " + JSON.stringify(mConfig));
    returnConfigToPebble();
});

Pebble.addEventListener("showConfiguration", function(e) {
    Pebble.openURL(mConfig.configureUrl);
});

Pebble.addEventListener("webviewclosed",
			function(e) {
			    var config = JSON.parse(e.response);
			    //console.log("Web view responded with: " + JSON.stringify(config));
			    saveLocalData(config);
			    //console.log("Local data saved as: " + JSON.stringify(mConfig));
			    returnConfigToPebble();
			}
		       );

function saveLocalData(config) {

    localStorage.setItem("bluetooth_vibe", parseInt(config.bluetooth_vibe));
    localStorage.setItem("hourly_vibe", parseInt(config.hourly_vibe));
    localStorage.setItem("battery_hand", parseInt(config.battery_hand));
    localStorage.setItem("charge_blink", parseInt(config.charge_blink));
    localStorage.setItem("inverted_colors", parseInt(config.inverted_colors));
    localStorage.setItem("minute_hands", parseInt(config.minute_hands));

    loadLocalData();

}
function loadLocalData() {
    
    mConfig.bluetooth_vibe = parseInt(localStorage.getItem("bluetooth_vibe"));
    mConfig.hourly_vibe = parseInt(localStorage.getItem("hourly_vibe"));
    mConfig.battery_hand = parseInt(localStorage.getItem("battery_hand"));
    mConfig.charge_blink = parseInt(localStorage.getItem("charge_blink"));
    mConfig.inverted_colors = parseInt(localStorage.getItem("inverted_colors"));
    mConfig.minute_hands = parseInt(localStorage.getItem("minute_hands"));
    mConfig.configureUrl = "http://thedadams.com/watchface/index.html";

    if(isNaN(mConfig.bluetooth_vibe)) {
	    mConfig.bluetooth_vibe = 1;
    }
    if(isNaN(mConfig.hourly_vibe)) {
	    mConfig.hourly_vibe = 0;
    }
    if(isNaN(mConfig.battery_hand)) {
	    mConfig.battery_hand = 1;
    }
    if(isNaN(mConfig.charge_blink)) {
	    mConfig.charge_blink = 1;
    }
    if(isNaN(mConfig.inverted_colors)) {
	    mConfig.inverted_colors = 0;
    }
    if(isNaN(mConfig.minute_hands)) {
	    mConfig.minute_hands = 0;
    }
}
function returnConfigToPebble() {
    Pebble.sendAppMessage({
	    "bluetooth_vibe":parseInt(mConfig.bluetooth_vibe),
	    "hourly_vibe":parseInt(mConfig.hourly_vibe),
	    "battery_hand":parseInt(mConfig.battery_hand),
	    "charge_blink":parseInt(mConfig.charge_blink),
        "inverted_colors":parseInt(mConfig.inverted_colors),
        "minute_hands":parseInt(mConfig.minute_hands)
    });
    //console.log("Message sent");
}