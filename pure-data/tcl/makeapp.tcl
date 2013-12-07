#  functions for embedding a patch into a standalone Mac OS X app

package provide makeapp 0.1

namespace eval ::makeapp:: {
    namespace export getpatchname
}

proc ::makeapp::promptreplace {appdir} {
    if {[file exists $appdir]} {
        set answer [tk_messageBox -message [concat overwrite $appdir "?"] \
                        -type yesno -icon question]
        return [string equal $answer "yes"]
    } else {
        return 1
    }
}

proc ::makeapp::createapp {appdir} {
    set pdapp_contents [file normalize [file join $::sys_guidir ".." ".."]]
    pdtk_post [_ "Copying: $pdapp_contents\n  -->\t$appdir/"]\n
    catch {
        exec -- chmod -R u+w $appdir
        file delete -force -- $appdir
    }
    file mkdir $appdir
    file copy -- $pdapp_contents "$appdir/"
    file rename -force -- $appdir/Contents/org.puredata.pdextended.default.plist \
        $appdir/Contents/org.puredata.pdextended.plist
}

proc ::makeapp::makeinfoplist {appdir} {
    regexp {.*/(.+)\.app} $appdir -> appname
    pdtk_post [format [_ "Setting up %s/Contents/Info.plist"]\n $appdir]
    set info_plist [open "$appdir/Contents/Info.plist" r]
    set info_plist_contents [read $info_plist]
    regsub -- {CFBundleName</key>.*?<string>Pd-.*extended.*<} $info_plist_contents \
        "CFBundleName</key>\n\t<string>$appname<" info_plist_contents
    set cfbundleversion [clock format [clock seconds] -format %Y.%m.%d]
    regsub -- {CFBundleVersion</key>.*?<string>.*?<} $info_plist_contents \
        "CFBundleVersion</key>\n\t<string>$cfbundleversion<" info_plist_contents
    regsub -- {org.puredata.pd.wish} $info_plist_contents \
        "org.puredata.pd.app.$appname" info_plist_contents
    regsub -- {<key>CFBundleDocumentTypes.+?</array>.+?</array>} \
        $info_plist_contents {} info_plist_contents
    regsub -- {<key>UTExportedTypeDeclarations.+</array>} $info_plist_contents \
        {} info_plist_contents
    close $info_plist
    set info_plist [open "$appdir/Contents/Info.plist" w]
    puts $info_plist $info_plist_contents
    close $info_plist		
}

proc ::makeapp::copycurrentpatch {appdir patch patchname isdir} {
    set extradir "$appdir/Contents/Resources/extra"
    file attributes $extradir -permissions u+w
    file mkdir "$extradir/app-auto-load"
    if {$isdir} {
        pdtk_post [format [_ "Copying: %s\n  -->\t$extradir/"]\n \
                       [file dirname $patch]]
        set patchdir [file normalize [file dirname $patch]]
        foreach file [glob -directory "$patchdir" -- * .*] {
            if {"$file"!="$appdir" && "$file"!="$patchdir/." && "$file"!="$patchdir/.."} {
                file copy -- $file "$extradir/app-auto-load/"
            }
        }
    } else {
        set embedded_patch "$extradir/app-auto-load/$patchname.pd"
        pdtk_post [_ "Copying: $patch\n  -->\t$embedded_patch"]\n
        file copy -- $patch $embedded_patch
    }
}

proc ::makeapp::getpatchname {mytoplevel} {
    set mytoplevel_path [wm attributes $mytoplevel -titlepath]
    if {$mytoplevel_path != ""} {
        return $mytoplevel_path
    } else {
        return ""
    }
}

proc ::makeapp::embedprefs {appdir patch_to_open} {
    pdtk_post [_ "Setting up $appdir/Contents/org.puredata.pdextended.plist"]\n
    set plist [open "$appdir/Contents/org.puredata.pdextended.plist" r]
    set new_plist [read $plist]
    close $plist
    regsub -- {flags</key>.*?<string>.*?<} $new_plist \
        "flags</key>\n\t<string>-open hcs/embed.pd -open $patch_to_open<" new_plist
    set plist [open "$appdir/Contents/org.puredata.pdextended.plist" w]
    puts $plist $new_plist
    close $plist		
}

proc ::makeapp::busypanel {appdir} {
    toplevel .makeapp
    wm title .makeapp [_ "Making App"]
    wm attributes .makeapp -topmost 1
    wm resizable .makeapp 0 0
    label .makeapp.label -text [format [_ "Making App in %s..."] $appdir]
    pack .makeapp.label -side top -fill both -ipadx 200 -ipady 100
}
