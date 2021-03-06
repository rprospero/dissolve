---
title: RemoveSpecies (Node)
linkTitle: RemoveSpecies
description: Remove species molecules from a box
---

{{< htable >}}
| | |
|--------|----------|
|Context|Generation|
|Name Required?|No|
|Branches|--|
{{< /htable >}}

## Overview

The `RemoveSpecies` node removes all molecules of one or more types of species from a configuration or, optionally, a selection of molecules as generated by one of the ["pick"-style nodes]({{< ref "picknode" >}}).

## Configuration

### Control Keywords

|Keyword|Arguments|Default|Description|
|:------|:--:|:-----:|-----------|
|`Selection`|`name`|--|Specify the name of a picking node - the molecules picked by that node will be deleted.|
|`Species`|`name`|--|Target species to remove - all molecules of the given species will be deleted from the configuration. More than one species name may be given to the keyword if it is desired to remove multiple species.|
