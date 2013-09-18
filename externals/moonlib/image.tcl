
namespace eval ::moonlib::image:: {
}

proc ::moonlib::image::configure {obj_id img filename} {
    if { [catch {$img configure -file $filename} fid]} {
        ::pdwindow::logpost $obj_id 1 "[image]: error reading '$filename':\n$fid\n"
    }
}

proc ::moonlib::image::create_photo {obj_id img filename} {
    if { [catch {image create photo $img -file $filename} fid]} {
        ::pdwindow::logpost $obj_id 1 "[image]: error reading '$filename':\n$fid\n"
    }
}

image create photo ::moonlib::image::noimage -data {
    R0lGODlhDwARANU8AAAAAP6xDv/yHP6kDP6oCv7FEP/pGUY6J/7KEP7CD//tG/6sCurp6FVLM+/v
    7vzhGP7SE/61DtfW05pwG/7ZFXFjOmNWM31yXv65Dv69D/nuIu+9Em5hRmlcO1FELv///1tILLWW
    I/b29v/kGOubD/7cFteMEYBdJoiAbpKMeJuYjWNXQF9UOrq4stmYEui/FoqEc3VoRZWQfMCBEtbG
    MO3VHe6qEObUOZiDQ+2wEcJ/EnluWf///wAAAAAAAAAAACH5BAEAADwALAAAAAAPABEAQAa1QJ5Q
    JIt1Kh1OSiTkNSAQgBRRSEgBgcChKWxdOLsWl5eiKQRoheFBQSQKExjvMsFErtLBYHIRqnAaaAIG
    axQvFio8Fg94ACWNGB4oIQWVCRFZCwQEeyhCDAcumpwDJgcMXBIVN4FqNSErEkIOCoGCBiNtCBsH
    DiwPhApqIyUQVRk5HhUUVx8AUVLOASAKGwhUx1mZJAoSFhgZAJgBmgCcILIOBzaYo3okvVwMKycz
    JjonLKhCQQA7
}
