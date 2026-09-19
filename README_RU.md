# ISB V100 Hub — русская версия

**ISB — Intelligent Systems Bureau**

Открытая платформа пользовательского уровня для **Tesla V100**, предназначенная для управления, совместимости, оптимизации и диагностики поверх установленного стека драйверов NVIDIA.

> ISB не заменяет kernel/user-mode драйвер NVIDIA. Установленный NVIDIA/Google-совместимый драйвер отвечает за доступ к оборудованию и интеграцию с ОС. ISB предоставляет пользовательский control plane, V100-специфичное определение возможностей, диагностику, оптимизацию, инструменты совместимости и управление внешними компонентами.

[English version / Английская версия](README.md)

## Что такое ISB?

Проект начинался как **V100 Driver Fixer**, а затем превратился в **control plane и платформу совместимости/расширений для Tesla V100**.

Главная цель:

**сделать Tesla V100 удобнее для использования, настройки, диагностики и производительной работы, не выдавая ISB за замену драйвера NVIDIA.**

Основной путь выглядит так:

Игра / приложение / вычислительная задача → ISB V100 Hub → providers и функциональные модули → базовый драйвер NVIDIA → Tesla V100.

## Основные возможности

### Control Center / Hub

Единая точка управления V100:

- точная идентификация GPU и варианта V100;
- состояние драйвера, runtime и графических API;
- capabilities и provenance;
- телеметрия;
- производительность и управление;
- диагностика;
- игры и совместимость;
- OptiScaler;
- отчёты и verification.

GUI и CLI используют одни и те же контракты Hub. GUI не должен напрямую обращаться к низкоуровневым providers.

### Performance и Optimize V100

Мониторинг и настройка V100 там, где установленный драйвер предоставляет необходимые management API:

- загрузка GPU;
- температура и энергопотребление;
- SM/HBM clocks и VRAM;
- ECC;
- PCIe и NVLink, если доступны;
- процессы;
- persistence mode;
- application clocks;
- compute mode;
- поддерживаемые power/performance controls;
- профили рабочих нагрузок.

Основной pipeline оптимизации:

**SCAN → DETECT → ANALYZE → PLAN → REVIEW → APPLY → VERIFY → RESULT**

Изменения проходят через:

**requested → applied → read-back → verified**

Неподтверждённое изменение никогда не должно отображаться как успешно применённое.

### Driver Doctor / FixEngine

- fingerprint оборудования, драйвера и API;
- детерминированная диагностика;
- правила известных проблем;
- обратимые workaround'ы;
- dry-run планы;
- явное применение изменений;
- проверка после изменения;
- rollback;
- детерминированные отчёты.

### Games

Планируемый V100-ориентированный Game Manager включает:

- поиск игр;
- определение executable и graphics API;
- compatibility profiles;
- настройки для конкретной игры;
- обнаружение применимости OptiScaler;
- безопасную оптимизацию;
- backup;
- rollback.

Неизвестная совместимость остаётся **UNKNOWN** и никогда автоматически не считается поддерживаемой.

### OptiScaler Manager

OptiScaler остаётся **внешним компонентом**. ISB управляет им, а не переписывает его:

- обнаружение версии и выбор совместимой версии;
- конфигурация;
- backup и hash verification;
- update и rollback/remove;
- provenance и license metadata.

### Graphics Enhancement

Программные графические возможности могут включать:

- spatial upscaling;
- масштабирование разрешения;
- sharpening;
- dynamic resolution;
- frame-pacing telemetry;
- отдельные настройки качества изображения и latency;
- интеграцию с внешним OptiScaler.

Экспериментальная часть может исследовать CUDA/Tensor neural upscaling, frame interpolation и software ray/lighting techniques. Это программные альтернативы, а не нативные RTX-возможности V100.

### Диагностика, отчёты и benchmarks

ISB предназначен для создания evidence-backed отчётов и воспроизводимых benchmark'ов для CUDA, Tensor Cores, HBM, graphics, PCIe/NVLink и сравнений before/after.

Отчёты сохраняют provenance и различают реальные, mock, unavailable и unverified данные.

## Модель capabilities

ISB разделяет:

1. **Hardware capability** — что физически содержит V100.
2. **Base-driver capability** — что предоставляет установленный driver/runtime.
3. **Observed capability** — что реально наблюдают providers.
4. **Verified capability** — что подтверждено evidence/tests.
5. **ISB capability** — что добавляет сам ISB поверх базового стека.

Состояния capability являются явными:

- AVAILABLE
- UNAVAILABLE
- UNKNOWN
- PERMISSION_DENIED
- ERROR

**UNKNOWN не равно UNAVAILABLE.**

Для V100 ISB не должен заявлять наличие RT Cores, отдельного Optical Flow Accelerator, MIG, аппаратного DLSS или других возможностей, которых нет у GV100/Volta. Программные реализации должны явно обозначаться как software.

## Driver Manager

Планируемый Driver Manager отвечает за жизненный цикл установленного базового драйвера:

- metadata каталога NVIDIA/Google;
- определение версии и ветки;
- compatibility matching;
- URL/artifact parser;
- download;
- checksum/signature verification;
- local cache;
- provenance;
- явную установку по действию пользователя.

ISB не должен молча заменять установленный драйвер или запускать произвольные скачанные installers.

## Release Builder и Installer

Планируется воспроизводимый pipeline релизов:

**tag → clean checkout → configure → build → test → package → SHA256/manifest → release notes → GitHub Release**

Целевые артефакты:

- Windows x64: ZIP и EXE;
- Linux x86_64: DEB и TAR.GZ.

Сгенерированные бинарники являются release artifacts и не должны коммититься в основной исходный код.

## Поддерживаемый диапазон V100

Основная цель квалификации:

**NVIDIA Tesla V100 SXM2 16 GB**

Отдельно отслеживаются V100 PCIe, V100 32 GB, multi-GPU конфигурации и headless compute с отдельной GPU для вывода изображения.

Начальный диапазон ОС:

- Windows 10/11 x64;
- Linux x86-64.

Legacy Windows/BSD — отдельная квалификация.

## Архитектура и документация

Репозиторий строится вокруг одного Hub/control plane с общими providers, FixEngine, compatibility, graphics, performance, diagnostics, verification и reporting слоями.

Перед архитектурными изменениями следует прочитать:

- [Architecture](docs/ARCHITECTURE.md)
- [Implementation plan](docs/IMPLEMENTATION_PLAN.md)
- [Roadmap status](docs/ROADMAP_STATUS.md)
- [Tasks](docs/TASKS.md)
- [Master specification](docs/MASTER_SPEC.md)
- [Master addendum](docs/MASTER_TZ_ADDENDUM.md)
- [Current implementation](docs/CURRENT_IMPLEMENTATION.md)
- [Driver and release pipeline](docs/DRIVER_AND_RELEASE_PIPELINE.md)
- [Agent guide](docs/AGENT_GUIDE.md)

## Текущее состояние реализации

Репозиторий находится в активной фазе разработки. Уже сформированы базовые common status/result/error contracts, определения возможностей V100/GV100, архитектура Hub/control plane, qualification/evidence gating, read-only NVML integration, контракты telemetry/tuning/optimization и deterministic mock slices, основы FixEngine, verification infrastructure, экспериментальные graphics/render-path foundations, граница интеграции OptiScaler, основы Control Center и архитектура Driver/Release pipeline.

Основная оставшаяся работа — переход от контрактов, mock-реализаций и частичных providers к полноценно интегрированному реальному стеку V100:

**M0 Build & stabilization → M1 Capability Engine → M2 real providers → M3 real telemetry → M4 tuning/Optimize V100 → M5 Driver Doctor → M6 graphics/render path → M7 Games/OptiScaler → M8 Driver Manager → M9 Release Builder/Installer → M10 Control Center → M11 benchmarks/evidence → M12 experimental graphics → физическая квалификация на V100.**

Актуальная карта implementation gaps находится в [docs/ROADMAP_STATUS.md](docs/ROADMAP_STATUS.md) и [docs/TASKS.md](docs/TASKS.md).

## Чего ISB не делает

ISB не добавляет магически аппаратные возможности, которых физически нет у V100.

ISB не должен:

- заменять kernel driver NVIDIA;
- выдавать software implementation за нативную RTX/DLSS/RT/OFA hardware feature;
- молча изменять online/anti-cheat игры;
- молча устанавливать произвольные внешние бинарники;
- выдавать mock-данные за результаты физической V100;
- превращать UNKNOWN в AVAILABLE.

Независимый исследовательский GV100 KMD/UMD driver остаётся изолированным в research/alternative-driver/ и не является зависимостью основного Hub.

## Принципы разработки

1. Hub first.
2. Evidence before claims.
3. Requested → applied → verified.
4. Unknown stays unknown.
5. Reversible changes by default.
6. GUI и CLI используют один control plane.
7. External components остаются external.
8. V100-specific behavior имеет приоритет.
9. Никаких silent global mutations.
10. Research code остаётся изолированным.

## Для разработчиков и AI-агентов

Перед архитектурными изменениями прочитайте MASTER_SPEC.md, MASTER_TZ_ADDENDUM.md, AGENT_GUIDE.md, CURRENT_IMPLEMENTATION.md, ROADMAP_STATUS.md и TASKS.md.

Не создавайте дублирующие подсистемы, если ответственность уже принадлежит существующему Hub, provider, FixEngine, capability или verification contract.

## Лицензирование

Лицензии проекта и provenance сторонних компонентов находятся в legal/.
