package com.efurture.minio.test;

import io.minio.*;
import io.minio.errors.*;
import io.minio.http.Method;

import java.io.IOException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.util.UUID;
import java.util.concurrent.TimeUnit;

public class MinioTest {

    public static void main( String[] args ) {

        try {
            doMinioTest();
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    public  static void doMinioTest() throws IOException, InvalidKeyException, InvalidResponseException, InsufficientDataException, NoSuchAlgorithmException, ServerException, InternalException, XmlParserException, ErrorResponseException {
        String url = "http://192.168.2.103:9000";
        String user = "minioadmin";
        String password  = "minioadmin";
        MinioClient minioClient =
                MinioClient.builder()
                        .endpoint(url)
                        .credentials(user, password)
                        .build();

        // Make 'asiatrip' bucket if not exist.
        String bucket = "asiatrip";
        boolean found = minioClient.bucketExists(BucketExistsArgs.builder().bucket(bucket).build());
        if (!found) {
            // Make a new bucket called 'asiatrip'.
            minioClient.makeBucket(MakeBucketArgs.builder().bucket(bucket).build());
        } else {
            System.out.println("Bucket 'asiatrip' already exists.");
        }


        String[] datas = {"/Users/efurture/Downloads/settings.xml", "/Users/efurture/Downloads/5c919e180001d70808320538.jpg",
        "/Users/efurture/照片/2019/11/IMG_4420.JPG"};

        int numOfFiles = 100*10000;

        for(int i=0; i<numOfFiles; i++){
            String file = datas[i%datas.length];
            String object = UUID.randomUUID().toString();
            int index = file.lastIndexOf('.');
            if(index >= 0){
                object += file.substring(index);
            }
            long start = System.currentTimeMillis();
            ObjectWriteResponse response = minioClient.uploadObject(
                    UploadObjectArgs.builder()
                            .bucket(bucket)
                            .object(object)
                            .filename(file)
                            .build());
           System.out.println(object + "used " + (System.currentTimeMillis() - start)  + " i = " + i);



           /**
            String fileUrl =
                    minioClient.getPresignedObjectUrl(
                            GetPresignedObjectUrlArgs.builder()
                                    .method(Method.GET)
                                    .bucket(bucket)
                                    .object(object)
                                    .expiry(6, TimeUnit.DAYS)
                                    .build());
            System.out.println(fileUrl);*/


        }

        /*

        ObjectWriteResponse response = minioClient.uploadObject(
                UploadObjectArgs.builder()
                        .bucket(bucket)
                        .object("m2_settings.xml")
                        .filename(datas[2])
                        .build());
        System.out.println(
                "'/home/user/Photos/asiaphotos.zip' is successfully uploaded as "
                        + "object 'asiaphotos-2015.zip' to bucket 'asiatrip'." + response.etag() + " response " + response.bucket()
        + " " + response.versionId() + " " + response.object());*/
    }
}