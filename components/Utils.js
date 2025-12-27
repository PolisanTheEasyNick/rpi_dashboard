function lerp(start, end, t) {
    return start + (end - start) * t
}

function lerpColor(c1, c2, t) {
    var clampedT = Math.max(0.0, Math.min(1.0, t))

    return Qt.rgba(lerp(c1.r, c2.r, clampedT), lerp(c1.g, c2.g, clampedT),
                   lerp(c1.b, c2.b, clampedT), 1)
}

function outsideTempColor(temp) {
    var stops = [{
                     "t": -10,
                     "c": Qt.rgba(0.2, 0.4, 1.0, 1)
                 }, // deep cold
                 {
                     "t": 0,
                     "c": Qt.rgba(0.3, 0.6, 1.0, 1)
                 }, // blue
                 {
                     "t": 10,
                     "c": Qt.rgba(0.3, 0.9, 0.9, 1)
                 }, // cyan
                 {
                     "t": 20,
                     "c": Qt.rgba(0.4, 0.9, 0.4, 1)
                 }, // green
                 {
                     "t": 25,
                     "c": Qt.rgba(1.0, 0.7, 0.2, 1)
                 }, // warm
                 {
                     "t": 30,
                     "c": Qt.rgba(1.0, 0.3, 0.3, 1)
                 } // hot
            ]

    if (temp <= stops[0].t)
        return stops[0].c
    if (temp >= stops[stops.length - 1].t)
        return stops[stops.length - 1].c

    for (var i = 0; i < stops.length - 1; ++i) {
        var a = stops[i]
        var b = stops[i + 1]

        if (temp >= a.t && temp <= b.t) {
            var t = (temp - a.t) / (b.t - a.t)
            return lerpColor(a.c, b.c, t)
        }
    }

    return Qt.rgba(1, 1, 1, 1)
}

function roomTempColor(val) {
    if (val < 18) {
        return Style.cold
    } else if (val <= 21) {
        var t = (val - 18) / (21 - 18)
        return lerpColor(Style.cold, Style.good, t)
    } else if (val <= 27) {
        var t2 = (val - 21) / (27 - 21)
        return lerpColor(Style.good, Style.warm, t2)
    } else {
        return Style.hot
    }
}
