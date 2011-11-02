function log(msg) {
    print("[Avatar app] " + msg);
}

function dc_set(ent, key, val) {
    var dc = ent.GetOrCreateComponent("EC_DynamicComponent");
    if (!dc.ContainsAttribute(key)) {
	// log("creating attr");
        dc.CreateAttribute("string", key);
	if (!dc.ContainsAttribute(key))
	    log("AddQVariantAttribute didn't work!");
    }
    dc.SetAttribute(key, val);
    // log("set attribute: " + key + ": " + val);
    // log("read back attribute: " + key + ": " + dc.GetAttribute(key));
}

function dc_get(ent, key) {
    var dc = ent.GetOrCreateComponent("EC_DynamicComponent");
    // log("read back (3) attribute: " + dc.GetAttribute("connectionID"));
    var val = dc.GetAttribute(key);
    // log("get attribute: " + key + ": " + val);
    // log("contains: " + dc.ContainsAttribute(key));
    return val;
}

function clear_av_connectionids(scene) {
    var av_ents = scene.GetEntitiesWithComponent("EC_Avatar");
    for (var i = 0; i < av_ents.length; i++) {
	var ent = av_ents[i];
	dc_set(ent, "connectionID", "");
    }
}
