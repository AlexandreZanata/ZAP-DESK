# Integração Reconner + OWASP ZAP

## Visão Geral

ZAP-DESK unifica dois motores complementares:

| Motor | Papel | Profundidade |
|-------|-------|--------------|
| **Reconner** | Mapeamento de superfície | Amplo e rápido |
| **OWASP ZAP** | Teste de aplicação | Profundo (spider, active scan) |

## Localização do Reconner

O Reconner v2.0.0 está vendored em `reconner/` (origem: THE-ULTMATE-RECONNER).

```
reconner/
├── reconner/cli.py      # Entry point CLI
├── reconner/runner.py   # ToolRunner — orquestração
├── reconner/reporter.py # summary.json
└── setup.py
```

## Como o ZAP-DESK invoca o Reconner

`ReconRunner` (C++) executa:

```bash
cd $RECONNER_DIR
PYTHONPATH=$RECONNER_DIR python3 -m reconner \
  -t <target> \
  -o <results_dir> \
  --quiet \
  [--fast] [--skip-nuclei] \
  --proxy http://127.0.0.1:8080
```

O flag `--quiet` pula o prompt interativo (a UI já exige checkbox de autorização).

## Bridge summary.json → ZAP

`ReconBridge` lê `summary.json` e extrai URLs de:

- `live_hosts.list`
- `subdomains.list`
- `gobuster_results[].url`

Para cada URL, chama `ZapClient::accessUrl()` para popular o site tree do ZAP.

## Modos de Operação

### 1. Recon isolado
Aba [RECON] → RUN RECON → resultados em `~/.local/share/ZAP-DESK/results/`

### 2. Feed manual
Após recon → FEED ZAP → ACTIVE SCAN

### 3. Full Pipeline
BOOT ZAP → RECON (via proxy) → FEED → ACTIVE SCAN automático

## Proxy Mode

Com `--proxy http://127.0.0.1:8080`, o tráfego HTTP do httpx, gobuster e nuclei passa pelo ZAP, permitindo inspeção manual paralela.

## Atualizações ZAP

1. Botão **CHECK ZAP UPDATE** na aba ZAP
2. Consulta GitHub API `zaproxy/zap/releases/latest`
3. Script `scripts/update-zap.sh` baixa e instala em `$ZAP_HOME`

Recomendado: verificar atualizações semanalmente (OWASP ZAP release cycle).

## Schema summary.json

```json
{
  "scan_info": { "timestamp", "target", "tool_versions" },
  "subdomains": { "total", "list" },
  "live_hosts": { "total", "list" },
  "gobuster_results": [{ "url", "path", "status" }],
  "nuclei_results": [{ "template", "severity", "url" }],
  "statistics": { "total_subdomains", "total_live_hosts" }
}
```

## Roadmap de Integração

- [ ] Fase ZAP nativa no ToolRunner Python
- [ ] Merge de alertas ZAP + nuclei na UI
- [ ] Export PDF unificado
- [ ] Agendamento de scans
- [ ] API local D-Bus para automação
