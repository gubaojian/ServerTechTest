package com.gubaojian.render;

/**
 * 不执行渲染是不知道具体数量的，因此必须执行渲染。
 * 如果对于长列表，不渲染。是不行的。list无法确定数据孙晓，
 * 正常v-for写到标签上面是不行的
 *
 * <cell v-for v-if>
 * </cell>
 * * <cell v-for v-if>
 *  * </cell>
 *
 *  1、v-for尝试失败,因为如果不渲染就不知道数据。这样大量数据就不行啦。
 *
 *
 *
 *  2、其它尝试 template不支持 v-for标签，
 *  3、对于list做特殊保护。 模板和渲染结果分离 listData="xxx" 这样。 pager也一样。
 *     仅支持放模板，模板不支持渲染，只支持延迟渲染。
 *     这样也会有问题，绑定时候会导致渲染时候view创建过多。影响滑动流畅性。难以限制模板中语句的使用。比如模板中使用循环来搞。
 *     viewtype count难以确定。 adapter view.  通过重写渲染方法进行处理。增加一个渲染缓存节点。 hash可以确定type。
 *
 *     渲染时机选择到什么时候？ getitemviewtype时进行？
 *     渲染数据如何使用及？
 *   通过if条件控制，不支持for循环，近通过支持if 进行区分。 （这样限制了开发体验，支持多）
 *   通过node属性控制是否支持for循环
 * 延迟渲染要保护好堆栈。和执行状态。可以方便最好回退。
 *
 *
 * 4、setdata 数据传输问题。动态性统一性，但是开发体验下降。
 *
 *
 * List<></>
 *
 * */
public class ListComponent extends  Component {
}
