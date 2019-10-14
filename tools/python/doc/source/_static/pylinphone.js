/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
$(function (){
var createList = function(selector){

    var ul = $('<ul>');
    var selected = $(selector);

    if (selected.length === 0){
        return;
    }

    selected.clone().each(function (i,e){

        var p = $(e).children('.descclassname');
        var n = $(e).children('.descname');
        var l = $(e).children('.headerlink');

        var a = $('<a>');
        a.attr('href',l.attr('href')).attr('title', 'Link to this definition');

        a.append(p).append(n);

        var entry = $('<li>').append(a);
        ul.append(entry);
    });
    return ul;
}


var c = $('<div style="min-width: 300px;">');

var ul0 = c.clone().append($('.submodule-index'))

customIndex = $('.custom-index');
customIndex.empty();
customIndex.append(ul0);

var x = [];
x.push(['Classes','dl.class > dt']);
x.push(['Functions','dl.function > dt']);
x.push(['Variables','dl.data > dt']);

x.forEach(function (e){
    var l = createList(e[1]);
    if (l) {        
        var ul = c.clone()
            .append('<p class="rubric">'+e[0]+'</p>')
            .append(l);
    }
    customIndex.append(ul);
});

});