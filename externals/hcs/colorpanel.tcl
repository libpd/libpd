
namespace eval ::hcs::colorpanel:: {
}

proc ::hcs::colorpanel::open {objectid initialcolor} {
    set color [tk_chooseColor -initialcolor $initialcolor]
    pdsend "$objectid callback $color"
}
