# Spreadsheet

## учебный проект
Упрощённое аналог существующих электронных таблиц. В ячейках таблицы могут быть текст или формулы. Формулы, как и в существующих решениях, могут содержать индексы ячеек.

Код лексического и синтаксического анализаторов, а также код для обхода дерева разбора генерировался через ANTLR.

Это итоговая работа курса по с++ Яндекс.Практикума.

Написана на с++

## реализация
**common.h** - Объявления вспомогательных структур. Классы возможных ошибок, интерфейсы ячеек и таблицы, а так же структура Position, содержащая адрес, константы с максимальными значениями колонок, линий и имеющая методы для работы с адресами.

Для пользователя ячейка таблицы задаётся своим индексом, то есть строкой вида “А1”, “С14” или “RD2”. В программе положение ячейки описывается позицией, то есть номерами её строки и столбца, начиная от 0, как это принято в С++. Например, индексу “А1” соответствует позиция (0, 0), а индексу “AB15” — позиция (14, 27). Все ячейки, кроме формульных и пустых, трактуются как текстовые

Значение ячеек бывает трех типов - текстовая, формальная и пустая. Задаются строкой текста.  Ячейка трактуется как формульная, если её строка начинается со знака "=". Пробелы перед знаком "=" не игнорируются. Строка, которая содержит только знак "=", формулой не считается, а считается текстовой.

**Sheet** - класс таблицы владеющий ячейками. Имеющий методы работы с ячейкой:
- Создания ячейки с указанным значением в обозначенной позиции SetCell(Position, std::string). Ячейка добавляется в таблицу только после того как успешно создана и проверена на циклические связи.
- Получения доступа к ячейке по позиции CellInterface* GetCell(Position).
- Проверки существования заданной ячейки PositionExists(Position).
- Удаления указанной ячейки ClearCell(Position).

Методы применимые ко всей таблицы в целом:
- Вычисление минимальной печатной области GetPrintableSize().
- Вывод в поток сосчитанных значений, для формул это результат вычислений PrintValues(std::ostream&). В случае текстовой - задающий ее текст, кроме случая, когда текст начинается с символа ' (апостроф). Тогда этот символ отсутствует.
- Вывод в поток текста заданного в ячейку PrintTexts(std::ostream& output). Для формульных это формула, очищенная от лишних скобок, но с ведущим знаком “=”. Для текстовых ячеек это текст, который пользователь задал в ячейку, то есть не очищенный от ведущих апострофов ';

Между заданными ячейками одной строки при выводе в поток ставиться табуляция, в конце строки - символ переноса строки.

**Cell** - класс ячейки. Хранит данные (в подклассах Impl), взаимосвязи и отвечает за корректное создание и очистку ячейки. В конструкторе необходимо передать ячейке ссылку на таблицу Cell(Sheet&). Затем уже данные ячейки задаются методом Set(std::string), который выброси исключение в случае некорректной формулы (нельзя вычислить или содержит адреса ячеек вне доступной области).

Когда ячейка задана формулой, создается класс формулы Formula, который создает и владеет FormulaAST - классом, отвечающим за работу с деревом разбора с генерированным ANTLR.

Cell имеет метод std::vector
GetReferencedCells() возвращающий адреса ячеек задействованных в формуле. Он необходим для проверки формулы на цикличность методом LoopCheck(const std::vector
, Position, std::set) (учитывает уже проверенные позиции, не ходит по ним повторно).

## оптимизация работы
Формулы могут представлять собой крайне ветвистую структуру, ссылающуюся на другие формулы. Для избежания повторного вычисления ранее вычисленных формул, в классе с формульными данными ячейки Impl::FormulaImpl добавлен кэш std::optional, если он заполнен, то вычисление не требуется и возвращается это значение double.

Реализация кэша потребовала сохранения взаимосвязей ячеек. В классе Cell хранятся два контейнера std::set с depends_ и broadcasts_, то есть с адресами ячеек от которых зависит данная и которые от нее зависят. При создании ячейки, после проверки на цикличность и вставки в таблицу необходимо заполнить контейнер depends_ и прописаться в качестве зависимой ячейки в broadcasts_ каждой ячейки того же списка depends_. Если по адресу еще не существует ячейка - создать пустую, которая в формуле трактуется нулем.

В случае изменения или удаления ячейки, необходимо пройтись по зависящим ячейкам broadcasts_ и далее по зависимом от них - обнулить кэшированное значение, требующее теперь пересчета, когда снова поступит запрос на значение. Так же надо выписаться из broadcasts_ ячеек от которых зависит. broadcasts_ не исчезает при удалении ячейки, а наследуется от старой к новой.

## cборка с помощью CMake

- Скачайте и установите [Java SE Runtime Environment 8](https://www.antlr.org) под вашу систему.
- Создайте папку для сборки программы.
- Перенесите в папку  antlr.jar (Complete ANTLR Java binaries jar не ниже 4.7.2-complete). Скачанный с официального сайта и с репозитория antlr так же скачайте [runtime для c++](https://github.com/antlr/antlr4/tree/master/runtime/Cpp), сохрани. паку antlr4_runtime с ним в папку проект.
- Введите команду: cmake<путь к папке программы>.
- Будут сгенерированы коды дерева ANTLR в<путь к папке программы>\antlr4cpp_generated_src\Formula.
- Введите команду: cmake --build<путь к папке программы>.
- После сборки в папке сборки появится исполняемый файл spreadsheet.exe.

Это лишь ядро электронной таблицы, не для прямого использования, но в main.cpp собраны всевозможные тесты на примере которых видно как обращаться к классам программы.
