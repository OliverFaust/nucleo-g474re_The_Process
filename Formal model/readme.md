# HelloProcess: Formal Model (CSP-M)

This directory contains the foundational **Communicating Sequential Processes (CSP)** formal specification for the minimal "Hello World" process. It provides a mathematical abstraction of a single independent thread of execution, isolating its behavioral logic from the low-level FreeRTOS task scheduling and C++ wrapper configurations (`CSProcess`). The model has been checked with [ProB](https://prob.hhu.de/).

## Model Overview

The model specifies a basic sequential unit of computation that interacts with its environment through an observable event:

1. **`channel print`:** Represents the observable action of outputting data (the `"Hello world"` string) to the system console.
2. **`HelloProcess`:** The core behavioral definition. It performs a `print` event and then sequences into a terminating state (`SKIP`).
3. **`HelloProcessMAIN`:** Syntactic entry point ensuring the model successfully terminates or matches execution requirements.

## CSP-M Specification

```csp
-- CSPm specification of the HelloProcess 
channel print
channel delay

HelloProcess = print -> delay -> HelloProcess

SYSTEM = HelloProcess
