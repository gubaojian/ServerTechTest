package org.example;

import com.alibaba.fastjson2.JSON;
import org.agrona.DirectBuffer;
import org.agrona.MutableDirectBuffer;
import org.agrona.concurrent.UnsafeBuffer;
import org.openjdk.jmh.annotations.*;
import org.openjdk.jmh.infra.Blackhole;
import org.openjdk.jmh.runner.Runner;
import org.openjdk.jmh.runner.RunnerException;
import org.openjdk.jmh.runner.options.Options;
import org.openjdk.jmh.runner.options.OptionsBuilder;


/**

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.*;
import java.util.concurrent.TimeUnit;

@State(Scope.Thread)
@BenchmarkMode(Mode.Throughput)
@OutputTimeUnit(TimeUnit.SECONDS)
@Warmup(iterations = 5, time = 1, timeUnit = TimeUnit.SECONDS)
@Measurement(iterations = 10, time = 1, timeUnit = TimeUnit.SECONDS)
@Fork(1)
public class SerializationBenchmark {

    // 测试数据
    private TestObject testObject;
    private MutableDirectBuffer sbeBuffer;
    private TestObjectEncoder sbeEncoder;
    private TestObjectDecoder sbeDecoder;
    private byte[] sbeSerializedData;
    private String jsonString;

    @Setup
    public void setup() throws Exception {
        // 初始化测试对象
        testObject = createTestObject();

        // 初始化SBE
        sbeBuffer = new UnsafeBuffer(ByteBuffer.allocate(1024));
        sbeEncoder = new TestObjectEncoder();
        sbeDecoder = new TestObjectDecoder();

        // 预序列化以初始化
        sbeSerializedData = serializeWithSBE();
        jsonString = JSON.toJSONString(testObject);
    }

    private TestObject createTestObject() {
        TestObject obj = new TestObject();
        obj.setName("John Doe");
        obj.setAge(30);
        obj.setActive(true);
        obj.setScore(95.5);

        List<String> hobbies = new ArrayList<>();
        hobbies.add("Reading");
        hobbies.add("Running");
        hobbies.add("Cooking");
        obj.setHobbies(hobbies);

        Map<String, Object> attributes = new HashMap<>();
        attributes.put("height", 180.5);
        attributes.put("weight", 75.2);
        attributes.put("married", true);
        obj.setAttributes(attributes);

        TestObject nested = new TestObject();
        nested.setName("Nested Object");
        nested.setActive(false);
        obj.setNestedObject(nested);

        return obj;
    }

    private byte[] serializeWithSBE() {
        sbeEncoder.wrap(sbeBuffer, 0);

        // 设置基本字段
        sbeEncoder.name(testObject.getName());
        sbeEncoder.age(testObject.getAge());
        sbeEncoder.isActive(testObject.isActive());
        sbeEncoder.score(testObject.getScore());

        // 设置hobbies列表
        TestObjectEncoder.Hobbies hobbiesEncoder = sbeEncoder.hobbiesCount(testObject.getHobbies().size());
        for (String hobby : testObject.getHobbies()) {
            hobbiesEncoder.next().value(hobby);
        }

        // 设置attributes映射
        TestObjectEncoder.Attributes attributesEncoder = sbeEncoder.attributesCount(testObject.getAttributes().size());
        for (Map.Entry<String, Object> entry : testObject.getAttributes().entrySet()) {
            attributesEncoder.next()
                    .key(entry.getKey())
                    .value(entry.getValue().toString());
        }

        // 设置嵌套对象
        if (testObject.getNestedObject() != null) {
            TestObjectEncoder.NestedObject nestedEncoder = sbeEncoder.nestedObject();
            nestedEncoder.name(testObject.getNestedObject().getName());
            nestedEncoder.isActive(testObject.getNestedObject().isActive());
        }

        return sbeBuffer.byteArray();
    }

    private TestObject deserializeWithSBE(byte[] data) {
        sbeDecoder.wrap(new UnsafeBuffer(data), 0, TestObjectDecoder.SCHEMA_VERSION, TestObjectDecoder.BLOCK_LENGTH);

        TestObject obj = new TestObject();
        obj.setName(sbeDecoder.name());
        obj.setAge(sbeDecoder.age());
        obj.setActive(sbeDecoder.isActive());
        obj.setScore(sbeDecoder.score());

        // 解析hobbies列表
        List<String> hobbies = new ArrayList<>();
        TestObjectDecoder.Hobbies hobbiesDecoder = sbeDecoder.hobbies();
        while (hobbiesDecoder.hasNext()) {
            hobbiesDecoder.next();
            hobbies.add(hobbiesDecoder.value());
        }
        obj.setHobbies(hobbies);

        // 解析attributes映射
        Map<String, Object> attributes = new HashMap<>();
        TestObjectDecoder.Attributes attributesDecoder = sbeDecoder.attributes();
        while (attributesDecoder.hasNext()) {
            attributesDecoder.next();
            attributes.put(attributesDecoder.key(), attributesDecoder.value());
        }
        obj.setAttributes(attributes);

        // 解析嵌套对象
        if (sbeDecoder.nestedObject().exists()) {
            TestObject nested = new TestObject();
            TestObjectDecoder.NestedObject nestedDecoder = sbeDecoder.nestedObject();
            nested.setName(nestedDecoder.name());
            nested.setActive(nestedDecoder.isActive());
            obj.setNestedObject(nested);
        }

        return obj;
    }

    @Benchmark
    public void sbeSerialization(Blackhole bh) {
        byte[] data = serializeWithSBE();
        bh.consume(data);
    }

    @Benchmark
    public void sbeDeserialization(Blackhole bh) {
        TestObject obj = deserializeWithSBE(sbeSerializedData);
        bh.consume(obj);
    }

    @Benchmark
    public void fastjsonSerialization(Blackhole bh) {
        String json = JSON.toJSONString(testObject);
        bh.consume(json);
    }

    @Benchmark
    public void fastjsonDeserialization(Blackhole bh) {
        TestObject obj = JSON.parseObject(jsonString, TestObject.class);
        bh.consume(obj);
    }

    @Benchmark
    public void compareDataSize(Blackhole bh) {
        byte[] sbeData = serializeWithSBE();
        String json = JSON.toJSONString(testObject);

        System.out.printf("SBE Size: %d bytes, JSON Size: %d bytes%n",
                sbeData.length, json.getBytes().length);

        bh.consume(sbeData);
        bh.consume(json);
    }

    public static void main(String[] args) throws RunnerException {
        Options opt = new OptionsBuilder()
                .include(SerializationBenchmark.class.getSimpleName())
                .build();

        new Runner(opt).run();
    }
}

// 测试对象类
class TestObject {
    private String name;
    private int age;
    private boolean isActive;
    private double score;
    private List<String> hobbies;
    private Map<String, Object> attributes;
    private TestObject nestedObject;

    // getters and setters
    public String getName() { return name; }
    public void setName(String name) { this.name = name; }

    public int getAge() { return age; }
    public void setAge(int age) { this.age = age; }

    public boolean isActive() { return isActive; }
    public void setActive(boolean active) { isActive = active; }

    public double getScore() { return score; }
    public void setScore(double score) { this.score = score; }

    public List<String> getHobbies() { return hobbies; }
    public void setHobbies(List<String> hobbies) { this.hobbies = hobbies; }

    public Map<String, Object> getAttributes() { return attributes; }
    public void setAttributes(Map<String, Object> attributes) { this.attributes = attributes; }

    public TestObject getNestedObject() { return nestedObject; }
    public void setNestedObject(TestObject nestedObject) { this.nestedObject = nestedObject; }
}

// SBE 编码器和解码器类 (示例，实际需通过SBE工具生成)
class TestObjectEncoder {
    // 实际代码需通过SBE XML schema生成
    // 此处为简化示例

    private MutableDirectBuffer buffer;
    private int offset;

    public static final int BLOCK_LENGTH = 32;
    public static final int TEMPLATE_ID = 1;
    public static final int SCHEMA_ID = 1;
    public static final int SCHEMA_VERSION = 0;

    public TestObjectEncoder wrap(MutableDirectBuffer buffer, int offset) {
        this.buffer = buffer;
        this.offset = offset;
        return this;
    }

    // 省略其他字段的编码方法...

    public Hobbies hobbiesCount(int count) {
        // 实现列表编码
        return new Hobbies();
    }

    public Attributes attributesCount(int count) {
        // 实现映射编码
        return new Attributes();
    }

    public NestedObject nestedObject() {
        // 实现嵌套对象编码
        return new NestedObject();
    }

    // 内部类：Hobbies列表编码器
    public final class Hobbies {
        private int index;
        private final int count;

        public Hobbies() {
            this.count = 0;
            this.index = -1;
        }

        public boolean hasNext() {
            return index + 1 < count;
        }

        public Hobbies next() {
            if (!hasNext()) {
                throw new IllegalStateException();
            }
            index++;
            return this;
        }

        public Hobbies value(String value) {
            // 编码字符串值
            return this;
        }
    }

    // 内部类：Attributes映射编码器
    public final class Attributes {
        // 实现类似Hobbies的映射编码逻辑
        public Attributes key(String key) { return this; }
        public Attributes value(String value) { return this; }
        public boolean hasNext() { return false; }
        public Attributes next() { return this; }
    }

    // 内部类：NestedObject编码器
    public final class NestedObject {
        public NestedObject name(String name) { return this; }
        public NestedObject isActive(boolean active) { return this; }
    }

    // 省略其他方法...
}

class TestObjectDecoder {
    // 实际代码需通过SBE XML schema生成
    // 此处为简化示例

    private DirectBuffer buffer;
    private int offset;
    private int actingBlockLength;
    private int actingVersion;

    public static final int BLOCK_LENGTH = 32;
    public static final int TEMPLATE_ID = 1;
    public static final int SCHEMA_ID = 1;
    public static final int SCHEMA_VERSION = 0;

    public TestObjectDecoder wrap(
            DirectBuffer buffer, int offset, int actingVersion, int actingBlockLength) {
        this.buffer = buffer;
        this.offset = offset;
        this.actingVersion = actingVersion;
        this.actingBlockLength = actingBlockLength;
        return this;
    }

    // 省略其他字段的解码方法...

    public Hobbies hobbies() {
        return new Hobbies();
    }

    public Attributes attributes() {
        return new Attributes();
    }

    public NestedObject nestedObject() {
        return new NestedObject();
    }

    // 内部类：Hobbies列表解码器
    public final class Hobbies {
        private int index;
        private final int count;

        public Hobbies() {
            this.count = 0;
            this.index = -1;
        }

        public boolean hasNext() {
            return index + 1 < count;
        }

        public Hobbies next() {
            if (!hasNext()) {
                throw new IllegalStateException();
            }
            index++;
            return this;
        }

        public String value() {
            return ""; // 解码字符串值
        }
    }

    // 内部类：Attributes映射解码器
    public final class Attributes {
        // 实现类似Hobbies的映射解码逻辑
        public String key() { return ""; }
        public String value() { return ""; }
        public boolean hasNext() { return false; }
        public Attributes next() { return this; }
    }

    // 内部类：NestedObject解码器
    public final class NestedObject {
        public String name() { return ""; }
        public boolean isActive() { return false; }
        public boolean exists() { return true; }
    }

    // 省略其他方法...
} */
