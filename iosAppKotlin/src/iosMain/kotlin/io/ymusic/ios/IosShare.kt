package io.ymusic.ios

import kmpcommon.uri.Uri
import kmpcommon.uri.filePath
import kotlinx.cinterop.ExperimentalForeignApi
import kotlinx.cinterop.useContents
import platform.CoreGraphics.CGRectMake
import platform.Foundation.NSURL
import platform.UIKit.UIActivityViewController
import platform.UIKit.popoverPresentationController

/** The system share sheet with [text], over the top view controller. */
@OptIn(ExperimentalForeignApi::class)
internal fun iosShareText(text: String) = iosShareItems(listOf(text))

internal fun iosShareFile(uri: String) {
    val path = Uri.parse(uri).filePath()
    iosShareItems(listOf(NSURL.fileURLWithPath(path)))
}

@OptIn(ExperimentalForeignApi::class)
private fun iosShareItems(items: List<Any>) {
    val host = topViewController() ?: return
    val sheet = UIActivityViewController(activityItems = items, applicationActivities = null)
    // iPad presents the sheet as a popover, which must be anchored or UIKit throws.
    sheet.popoverPresentationController?.let { popover ->
        val view = host.view
        popover.sourceView = view
        view.bounds.useContents { popover.sourceRect = CGRectMake(size.width / 2, size.height / 2, 0.0, 0.0) }
        popover.permittedArrowDirections = 0u
    }
    host.presentViewController(sheet, animated = true, completion = null)
}
