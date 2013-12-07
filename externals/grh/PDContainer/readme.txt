PDContainer - by Georg Holzmann <grh@mur.at>, 2004-2007


--------------------------------license---------------------------------------

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

In the official PDContainer distribution, the GNU General Public License is
in the file gpl.txt


------------------------------PDContainer-------------------------------------

::: GOAL OF THE LIBRARY :::
This library was made for algorithmic composition and of course for all other algorithms. 
I came into troubles with making bigger musical structures in PD with send-receive pairs, 
arrays, etc. So I tried to make it possible, to have access to some storage in a whole patch.

::: DATASTRUCTURES :::
As storage datastructures I implemented the C++ STL (Standard Template Library) 
Containers in PD. Currently following datastructures are available 
(prefixed with h_): h_map, h_multimap, h_set, h_multiset, h_vector, h_list, h_deque, 
h_queue, h_priority_queue and h_stack.

::: NAMESPACES :::
For communication I use namespaces. Every Container with the same namespace 
(and the same container type) has access to the same data. So you can modify and get 
this data everywhere in the patch. For local namespaces use names with $0.

::: DATATYPES :::
In the containers you can save all of the build-in datatypes: lists, floats, symbols and pointers.

::: SAVE/LOAD :::
All the data of all containers can be saved to disk as XML or textfile. So you can also manually edit the file with an editor (which is sometimes much faster) and then load it in PD into a container. You can also load data from other containers. 
Please use the XML fileformat if possible, because it's easier to edit in an external editor and the XML parser is much more stable.
