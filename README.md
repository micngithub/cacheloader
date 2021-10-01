# qml_cacheloader
This code was created to reduce the load due to repeated qml loads by using cached quickitem without rereading the qml once read.

It was prepared to reduce the cpu load generated when two qmls were repeatedly loaded using a qml Loader.

The following events occur when the source property of the qml Loader is changed.
 1) Delete qquickitem objects for existing sources.
 2) Create qquickitem objects for new sources.

When repetitive qml loading is required according to the development situation, there is a problem that the process is repeated to cause a cpu load.

Therefore, when a qquickitem matching qml is created, the qquickitem is stored, and when there is a loading request for qml again, the existing qquickitem is delivered to reduce the cpu load.

Since the code was created to replace and use the qml Loader, we tried to go to the same structure as Loader's property as much as possible.

Memory leakage may occur because there is no delete code for the generated object in the current code.
However, this part is a problem that can be solved by adding it directly by the developer creating a deletion function, so I do not intend to add it in the future.

The attached image is also applied to the mit license.
thanks

@micn
