// compat
function fmod(a, b) { return a % b; }
function fabs(x) { return Math.abs(x); }

function gb(x) {
    return 2.0 * x - 1.0;
}

function gu(x) {
    return 0.5 * x + 0.5;
}

function glin(x, a1) {
    return a1 * x;
}

function glin(x, a1, a0) {
    return a1 * x + a0;
}

function mod1(x) {
    return fmod(x, 1.0);
}

function modm(x, m) {
    return fmod(x, m);
}

function gramp(x, a1, a0) {
    return mod1(glin(x, a1, a0));
}

function gtri(x) {
    return fabs(gb(x));
}

function gtri(x, a1, a0) {
    return mod1(glin(fabs(gb(x)), a1, a0));
}

function stri(x) {
    if (x < 0.5) {
        return 2.0 * x;
    } else {
        return 2.0 - 2.0 * x;
    }
}

function gpulse(x, w) {
    return x < w ? 0.0 : 1.0;
}

function gvslope(x, w) {
    return x < w ? x : (x - w) / (1.0 - w);
}

function svtri(x, w) {
    return gb(x) - gb(fabs(x - w));
}

function gvtri(x, w, a1, a0) {
    return mod1(glin(svtri(x, w), a1, a0));
}

function gripple(x, m) {
    return x + fmod(x, m);
}

function gripple2(x, m1, m2) {
    return fmod(x, m1) + fmod(x, m2);
}