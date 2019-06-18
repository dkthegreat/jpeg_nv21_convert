package com.le.camera.jpegdecenc.test;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

import com.le.camera.jpegdecenc.R;

import android.app.Activity;
import android.os.Bundle;


public class MainActivity extends Activity {
	
	public byte[] readBytes(String filePath) throws IOException {  
        File file = new File(filePath);  
        long fileSize = file.length();  
        if (fileSize > Integer.MAX_VALUE) {  
            System.out.println("file too big...");  
            return null;  
        }  
        FileInputStream fi = new FileInputStream(file);  
        byte[] buffer = new byte[(int) fileSize];  
        int offset = 0;  
        int numRead = 0;  
        while (offset < buffer.length  
        && (numRead = fi.read(buffer, offset, buffer.length - offset)) >= 0) {  
            offset += numRead;  
        }  
        // 确保所有数据均被读取  
        if (offset != buffer.length) {  
        throw new IOException("Could not completely read file "  
                    + file.getName());  
        }  
        fi.close();  
        return buffer;  
    }  
	
	void writeBytes(String strPathName, byte[] data) {
		File file = new File(strPathName);
		FileOutputStream fos;
		try {
			fos = new FileOutputStream(file);
			fos.write(data);
			fos.close();
		} catch (Exception e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		try {
			byte []fileIn = readBytes("/sdcard/1.jpg");
			writeBytes("/sdcard/2.jpg", fileIn);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		setContentView(R.layout.activity_main);
	}
	
	//
	public native static int ntvJpegToNV21(byte[] jpgData, int width, int height, byte[]nv21Data);
	
	//
	public native static byte[] ntvNV21ToJpeg(byte[] nv21Data, int width, int height);
}
