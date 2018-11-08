Android portability
===============
Video
-----

To be able to display remote and locally captured video streams on Android, you have to give the Core 2 TextureViews.
These objects must be given using :java:func`setNativeVideoWindowId` and java:func`setNativePreviewWindowId`.
Here's the complete code:

.. code-block:: java

	TextureView mVideoView = view.findViewById(R.id.videoSurface);
	TextureView mCaptureView = view.findViewById(R.id.videoCaptureSurface);

	LinphoneManager.getLc().setNativeVideoWindowId(mVideoView);
	LinphoneManager.getLc().setNativePreviewWindowId(mCaptureView);