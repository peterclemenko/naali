function log(msg) {
    print("[Avatar app] " + msg);
}

function dc_set(ent, key, val) {
    var dc = ent.GetOrCreateComponentRaw("EC_DynamicComponent");
    if (!dc.GetAttribute(key))
        dc.AddQVariantAttribute(key);
    dc.SetAttribute(key, val);
}

function dc_get(ent, key) {
    var dc = me.GetOrCreateComponentRaw("EC_DynamicComponent");
    return dc.GetAttribute(key);
}

function clear_av_connectionids(scene) {
    var av_ents = scene.GetEntitiesWithComponentRaw("EC_Avatar");
    for (var i = 0; i < av_ents.length; i++) {
	var ent = av_ents[i];
	dc_set(ent, "connectionID", "");
    }
}
