Android portability
===================
Video
-----

To be able to display remote and locally captured video streams on Android, you have to give the Core 2 TextureViews.
These objects must be given using :java:meth:`setNativeVideoWindowId` and :java:meth:`setNativePreviewWindowId`.
Here's the complete code:

.. code-block:: java

	Core mCore; // Get it the way you prefer

	TextureView mVideoView = view.findViewById(R.id.videoSurface);
	TextureView mCaptureView = view.findViewById(R.id.videoCaptureSurface);

	mCore.setNativeVideoWindowId(mVideoView);
	mCore.setNativePreviewWindowId(mCaptureView);
