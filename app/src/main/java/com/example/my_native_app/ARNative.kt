package com.example.my_native_app

import android.app.Activity
import android.content.Context

object ARNative {
    init {
        System.loadLibrary("my_native_app")
        println("SANJU : ARNative init - ${Thread.currentThread().name}")
    }

    external fun onCreate(context: Context, activity : Activity)
    external fun onResume(activity: Activity)
    external fun onPause()
    external fun onSurfaceCreated()
    external fun onDrawFrame(width : Int, height : Int, displayRotation : Int)
    external fun onTouch(x : Float, y : Float)

    external fun onRotateCube(degrees : Float)
    external fun onScaleCube(scale : Float)
    external fun onTranslateCube(x : Float, y : Float, z : Float)
}