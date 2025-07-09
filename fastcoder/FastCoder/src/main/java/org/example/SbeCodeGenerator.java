package org.example;



import uk.co.real_logic.sbe.SbeTool;
import uk.co.real_logic.sbe.ir.Ir;
import uk.co.real_logic.sbe.xml.IrGenerator;
import uk.co.real_logic.sbe.xml.MessageSchema;
import uk.co.real_logic.sbe.xml.ParserOptions;
import uk.co.real_logic.sbe.xml.XmlSchemaParser;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

public class SbeCodeGenerator {
    public static void main(String[] args) throws Exception {
        // 指定 SBE XML 模式文件
        File schemaFile = new File("src/main/resources/test-object.sbe.xml");

        // 创建解析选项
        ParserOptions options = ParserOptions.builder()
                .stopOnError(true)
                .build();

        // 使用 XmlSchemaParser 解析 XML 模式（传入 options）
        final MessageSchema schema = new XmlSchemaParser().parse(new FileInputStream(schemaFile), options);

        // 生成中间表示 (IR)
        final IrGenerator irGenerator = new IrGenerator();
        final Ir ir = irGenerator.generate(schema);

        // 配置并执行代码生成
        String outputDir = "src/generated/java";
        String targetLanguage = "Java"; // 目标语言参数

        SbeTool.generate(ir, outputDir, targetLanguage);
        System.out.println("SBE 代码生成完成！");
    }
}