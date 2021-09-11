package com.gubaojian.render;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Component {
    private String tag = null;
    private Map<String, String> attrs = new HashMap<>();
    private Map<String, String> events = new HashMap<>();
    private Map<String, Object> expressions = new HashMap<>();
    private Map<String,Object> state = new HashMap<>();
    private boolean isReachable = true;

    private List<Component> childrens = null;


    public void render(Object context, Object state){
        System.out.println("render");

        if(childrens != null){
            for(int i=0; i<childrens.size(); i++){
                Component component = childrens.get(i);
                component.doForLoop(context, state);
                component.render(context, state);
            }
        }
    }

    public void doForLoop(Object context, Object state){

    }

    public void doIf(Object context, Object state){
        Object ifEL = expressions.get("if");
        if(ifEL == null){
            return;
        }

    }


    public Map<String, String> getAttrs() {
        return attrs;
    }

    public void setAttrs(Map<String, String> attrs) {
        this.attrs = attrs;
    }

    public Map<String, String> getEvents() {
        return events;
    }

    public void setEvents(Map<String, String> events) {
        this.events = events;
    }

    public Map<String, Object> getExpressions() {
        return expressions;
    }

    public void setExpressions(Map<String, Object> expressions) {
        this.expressions = expressions;
    }

    public Map<String, Object> getState() {
        return state;
    }

    public void setState(Map<String, Object> state) {
        this.state = state;
    }

    public void add(Component component) {
        if(childrens == null){
            childrens = new ArrayList<>();
        }
        childrens.add(component);
    }

    public List<Component> getChildrens() {
        return childrens;
    }

    public void setChildrens(List<Component> childrens) {
        this.childrens = childrens;
    }

    public String getTag() {
        return tag;
    }

    public void setTag(String tag) {
        this.tag = tag;
    }
}
