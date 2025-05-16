package com.example.my_native_app

import android.Manifest
import android.content.Context
import android.content.res.Configuration
import android.graphics.Camera
import android.os.Bundle
import android.view.MotionEvent
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.ActivityResultLauncher
import androidx.activity.result.contract.ActivityResultContracts
import androidx.activity.viewModels
import androidx.compose.foundation.gestures.detectTapGestures

import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonColors
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.focus.focusModifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import androidx.lifecycle.lifecycleScope
import androidx.lifecycle.viewModelScope
import com.google.ar.core.Frame
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.SharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking

class MainActivity : ComponentActivity() {
    val cameraPermissionViewModel : CameraPermissionViewModel by viewModels {
        CameraPermissionViewModelFactory(application)
    }

    private val cameraPermissionRequestLauncher : ActivityResultLauncher<String> =
        registerForActivityResult(ActivityResultContracts.RequestPermission()) { isGranted : Boolean ->
            cameraPermissionViewModel.onPermissionResult(isGranted)
        }

    private lateinit var arSurfaceView: ARSurfaceView

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        println("SANJU : onCreate")

        cameraPermissionViewModel.checkCameraPermission()
        ARNative.onCreate(this, this)

        enableEdgeToEdge()
        setContent {
            val permissionGranted by cameraPermissionViewModel.isCameraPermissionGranted.collectAsState()
            Box(
                modifier = Modifier.fillMaxSize(),
                contentAlignment = Alignment.Center
            ) {
                if(permissionGranted) {
                        ArViewWithTouch()
//                        Button(onClick = {}) { Text("Click Here") }
                } else {
                    Button(onClick = {
                        cameraPermissionViewModel.requestCameraPermission {
                            cameraPermissionRequestLauncher.launch(Manifest.permission.CAMERA)
                        }
                    }) {
                        Text("Request Camera Permission")
                    }
                }
            }
        }
    }

    override fun onPause() {
        super.onPause()
        println("SANJU : onPause() ${Thread.currentThread().name}")
        ARNative.onPause()
        if(::arSurfaceView.isInitialized) {
            arSurfaceView.onPause()
        }
    }

    override fun onResume() {
        super.onResume()
        ARNative.onResume(this)
        if(::arSurfaceView.isInitialized) {
            arSurfaceView.onResume()
        }
    }

    override fun onConfigurationChanged(newConfig: Configuration) {
        super.onConfigurationChanged(newConfig)
        println("SANJU : Configuration Changed")
    }

    override fun onDestroy() {
        super.onDestroy()

    }

//    override fun onTouchEvent(event: MotionEvent?): Boolean {
//        if(event?.action == MotionEvent.ACTION_DOWN) {
//            val x_val = event.x
//            val y_val = event.y
//            // Send the x and y to the c++ side
//            ARNative.onTouch(x_val, y_val)
//        }
//        return true
//    }

    @Composable
    fun ArViewWithTouch() {
        val context : Context = LocalContext.current

        Box(
            modifier = Modifier
                .fillMaxSize()
                .pointerInput(Unit) {
                    detectTapGestures { offset: Offset ->
                        ARNative.onTouch(offset.x, offset.y)
                    }
                }
        ) {
            AndroidView(
                factory = {
                    ARSurfaceView(context).also { arSurfaceView = it }
                },
                modifier = Modifier.fillMaxSize()
            )

            Column(
                modifier = Modifier
                    .align(Alignment.BottomCenter)
                    .pointerInput(Unit) {
                        detectTapGestures { offset : Offset ->
                            ARNative.onTouch(offset.x, offset.y)
                        }
                    }
            ) {
                Row(
                    modifier = Modifier.align(Alignment.CenterHorizontally),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Button(
                        onClick = { ARNative.onRotateCube(-5f) },
                        modifier = Modifier.size(100.dp)
                    ) { Text("A") }
                    Spacer(modifier = Modifier.width(20.dp))
                    Button(
                        onClick = { ARNative.onRotateCube(5f) },
                        modifier = Modifier.size(100.dp)
                    ) { Text("C") }
                }
                Spacer(modifier = Modifier.height(20.dp))
                Row(
                    modifier = Modifier.align(Alignment.CenterHorizontally),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Button(
                        onClick = { ARNative.onTranslateCube(-0.01f, 0.0f, 0.0f) },
                        modifier = Modifier.size(80.dp)
                    ) { Text("Left") }
                    Spacer(modifier = Modifier.width(20.dp))
                    Button(
                        onClick = { ARNative.onTranslateCube(0.01f, 0.0f, 0.0f) },
                        modifier = Modifier.size(80.dp)
                    ) { Text("Right") }
                    Spacer(modifier = Modifier.width(20.dp))
                    Button(
                        onClick = { ARNative.onTranslateCube(0.0f, 0.0f, -0.01f) },
                        modifier = Modifier.size(80.dp)
                    ) { Text("Forward") }
                    Spacer(modifier = Modifier.width(20.dp))
                    Button(
                        onClick = { ARNative.onTranslateCube(0.0f, 0.0f, 0.01f) },
                        modifier = Modifier.size(80.dp)
                    ) { Text("Back") }
                }
                Spacer(Modifier.height(20.dp))
            }
        }
    }
}

