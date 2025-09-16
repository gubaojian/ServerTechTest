package org.efurture.test;

import com.ibm.icu.text.BreakIterator;
import com.ibm.icu.util.ULocale;

//TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or
// click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
public class Main {
    public static void main(String[] args) {
        // 待分词文本（中英日混合）
        String text = "ICU4J 77.1 支持中文、English和日本語Sqlite分com.taobao.efurture词。JAVA-Websocket 与 MySQL 相比，SQLite 更适合嵌入式场景，无需独立服务";

        // 创建单词分词器（使用默认语言环境，自动识别文本语言）
        BreakIterator wordIterator = BreakIterator.getWordInstance(ULocale.ROOT);
        wordIterator.setText(text);

        // 执行分词并输出结果
        int start = wordIterator.first();
        int end = wordIterator.next();

        System.out.println("基础单词分词结果：");
        while (end != BreakIterator.DONE) {
            String token = text.substring(start, end).trim();
            if (!token.isEmpty()) { // 过滤空字符（如空格、制表符）
                System.out.printf("位置 [%d-%d]: %s%n", start, end, token);
            }
            start = end;
            end = wordIterator.next();
        }
    }
}