#!/usr/bin/env zsh

BASE_ID="com.amyinorbit"

function sign {
    TARGET="$1"
    PLUGIN="$TARGET/mac_x64/$TARGET.xpl"
    BUNDLE_ID="$BASE_ID.$TARGET"
    
    CERT=$(security find-certificate -Z -c "Developer ID Application:" | \
        grep "SHA-1" | \
        awk 'NF { print $NF }')
    
    echo "Signing $PLUGIN ($BUNDLE_ID)..."
	codesign "$PLUGIN" \
        --sign "$CERT" \
        --force \
        --options runtime \
        --timestamp \
        --identifier "$BUNDLE_ID"
}


function notarize {
    TARGET="$1"
    PLUGIN="$1/mac_x64/$1.xpl"
    ZIPFILE="$(mktemp -u /tmp/$TARGET.XXXXXX).zip"
    PLISTBUDDY="/usr/libexec/PlistBuddy"
    
    SUBMIT_LOG="notarize/submission.plist"
    RESULT_LOG="notarize/notarization.plist"
    
    echo "Creating /notarize..."
    mkdir -p notarize
    
    echo "Packaging $PLUGIN for notarization..."
    zip "$ZIPFILE" "$PLUGIN" > /dev/null
    
    echo "Submitting notarization request..."
	xcrun notarytool submit "$ZIPFILE" \
		--keychain-profile AC_PASSWORD \
        --output-format plist > "$SUBMIT_LOG" || exit 1
    ID=$($PLISTBUDDY -c "Print :id" "$SUBMIT_LOG")
    
    xcrun notarytool wait "$ID" \
        --keychain-profile AC_PASSWORD || exit 1
    
    echo "Notarization complete"
    xcrun notarytool info "$ID" \
        --keychain-profile AC_PASSWORD \
        --output-format plist > "$RESULT_LOG" || exit 1
    
    STATUS=$($PLISTBUDDY -c "Print :status" "$RESULT_LOG")
    
    if [[ "$STATUS" == "Accepted" ]]; then
        echo "Notarization successful"
    else
        echo "Notarization error\nCheck notarize/notarization.plist for more info"
    fi
    
    rm "$ZIPFILE"
}


function pack {
    TARGET="$1"
    GIT_REV=$(git describe --tags --always)
    BUILD_ID="$(date +"%m%d")-$GIT_REV"
    BUILD_ZIP="htrack-$BUILD_ID.zip"
    
    if [[ "$(uname)" == "Darwin" ]]; then
        sign "$TARGET"
        notarize "$TARGET"
    fi
    
    echo "Packaging"
    zip -rq $BUILD_ZIP "$TARGET"
    rm -f $INFO_FILE
}


pack "htrack"