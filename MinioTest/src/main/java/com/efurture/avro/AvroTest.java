package com.efurture.avro;

import com.alibaba.fastjson.JSON;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.dataformat.avro.AvroMapper;
import com.fasterxml.jackson.dataformat.avro.AvroSchema;
import org.apache.avro.Schema;
import org.junit.Test;

import java.util.UUID;

public class AvroTest {

    @Test
    public void testAvro() throws JsonProcessingException {
        String SCHEMA_JSON = "{\n"
                +"\"type\": \"record\",\n"
                +"\"name\": \"Employee\",\n"
                +"\"fields\": [\n"
                +" {\"name\": \"name\", \"type\": \"string\"},\n"
                +" {\"name\": \"age\", \"type\": \"int\"},\n"
                +" {\"name\": \"emails\", \"type\": {\"type\": \"array\", \"items\": \"string\"}},\n"
                +" {\"name\": \"boss\", \"type\": [\"Employee\",\"null\"]}\n"
                +"]}";
        Schema raw = new Schema.Parser().setValidate(true).parse(SCHEMA_JSON);
        AvroSchema schema = new AvroSchema(raw);
        AvroMapper mapper = new AvroMapper();

        Employee empl = new Employee();

        empl.name = UUID.randomUUID().toString();
        empl.age = 20;
        empl.emails = new String[]{"787277208@qq.com"};

        byte[] avroData = mapper.writer(schema)
                .writeValueAsBytes(empl);

        System.out.println(avroData.length);

        System.out.println(JSON.toJSONString(empl).getBytes().length);

    }
}

class Employee
{
    public String name;
    public int age;
    public String[] emails;
    public Employee boss;
}
