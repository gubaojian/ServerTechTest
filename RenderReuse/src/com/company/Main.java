package com.company;

import com.gubaojian.render.CellComponent;
import com.gubaojian.render.Component;
import com.gubaojian.render.ListComponent;

import java.util.*;

public class Main {

    public static void main(String[] args) {
        List<Object> datas = new ArrayList<Object>();
        for(int i=0; i<1000; i++){
            Map<String, String> map = new HashMap<String, String>();
            map.put("text", "text" + i);
            map.put("type", "item");
        }
        for(int i=0; i<1000; i++){
            Map<String, String> map = new HashMap<String, String>();
            map.put("text", "text" + i);
            map.put("type", "shop");
        }

        ListComponent list = new ListComponent();


        list.add(createCell());
        list.add(createCell());



        Stack<String> state = new Stack<String>();
        list.render(datas, state);
    }

    private static CellComponent createCell(){
        CellComponent cell = new CellComponent();
        Component component = new Component();
        component.setTag("text");
        component.getExpressions().put("if", "item");
        cell.add(component);
        Component image = new Component();
        component.setTag("image");
        component.getExpressions().put("if", "shop");

        cell.getExpressions().put("for", 2000);
        return cell;
    }


}
