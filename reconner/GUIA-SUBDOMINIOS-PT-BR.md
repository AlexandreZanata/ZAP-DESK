# 🔍 Guia Completo: Descoberta de Subdomínios

**Alvo de Teste:** `http://177.69.195.4/`

---

## 📍 Localização das Ferramentas no Seu Sistema

### Gobuster
- **Localização:** `/usr/local/bin/gobuster`
- **Versão:** `3.8.2`
- **Status:** ✅ Instalado e funcionando

```bash
which gobuster        # → /usr/local/bin/gobuster
gobuster --version    # → gobuster version 3.8.2
```

### SecLists
- **Localização:** `/usr/share/seclists/`
- **Status:** ✅ Instalado e funcionando

```bash
ls /usr/share/seclists/
# Ai  Discovery  Fuzzing  Miscellaneous  Passwords  Payloads  Usernames  Web-Shells
```

### Wordlists Confirmadas no Seu Sistema

```bash
# DNS (subdomínios)
/usr/share/seclists/Discovery/DNS/subdomains-top1million-5000.txt
/usr/share/seclists/Discovery/DNS/subdomains-top1million-20000.txt
/usr/share/seclists/Discovery/DNS/subdomains-top1million-110000.txt
/usr/share/seclists/Discovery/DNS/combined_subdomains.txt
/usr/share/seclists/Discovery/DNS/dns-Jhaddix.txt

# Web-Content (diretórios)
/usr/share/seclists/Discovery/Web-Content/common.txt
/usr/share/seclists/Discovery/Web-Content/raft-small-directories.txt
/usr/share/seclists/Discovery/Web-Content/raft-medium-directories.txt
/usr/share/seclists/Discovery/Web-Content/raft-large-directories.txt
/usr/share/seclists/Discovery/Web-Content/raft-small-files.txt
/usr/share/seclists/Discovery/Web-Content/raft-medium-files.txt
/usr/share/seclists/Discovery/Web-Content/DirBuster-2007_directory-list-2.3-small.txt
/usr/share/seclists/Discovery/Web-Content/DirBuster-2007_directory-list-2.3-medium.txt
/usr/share/seclists/Discovery/Web-Content/DirBuster-2007_directory-list-2.3-big.txt
```

---

## 🔍 Resultado do Scan em `http://177.69.195.4/`

### Informações do Servidor (detectadas)

```
Server: Microsoft-IIS/10.0
X-Powered-By: ASP.NET
```

> **Importante:** É um servidor **Windows IIS com ASP.NET**. Use extensões `.asp`, `.aspx` nos scans!

### Descobertas do Scan Rápido (`common.txt`)

| Caminho | Status | Observação |
|---------|--------|------------|
| `/aspnet_client/` | 301 → redirect | Diretório padrão do IIS/ASP.NET |

### Como Interpretar os Status Codes

| Status | Significado |
|--------|-------------|
| **200** | ✅ Encontrado e acessível |
| **301/302** | 🔄 Redirecionamento (pode ser interessante) |
| **403** | 🔒 Existe mas acesso negado |
| **401** | 🔑 Requer autenticação |
| **500** | ⚠️ Erro do servidor (pode revelar info) |
| **404** | ❌ Não encontrado (ignorado por padrão) |

---



**⚠️ ATENÇÃO:** Você forneceu um **endereço IP** (`177.69.195.4`), mas a descoberta de subdomínios funciona com **domínios** (ex: `example.com`).

### Por que isso importa?

- **Subdomínios** são nomes como: `api.example.com`, `admin.example.com`, `dev.example.com`
- **IPs** não têm subdomínios — eles são endereços numéricos diretos
- Para descobrir subdomínios, você precisa do **nome de domínio** associado ao IP

### Como descobrir o domínio de um IP?

```bash
# Reverse DNS lookup
nslookup 177.69.195.4

# Ou usando host
host 177.69.195.4

# Ou usando dig
dig -x 177.69.195.4
```

---

## 🛠️ Opção 1: Descobrir Subdomínios (se você tiver o domínio)

Se você descobrir que `177.69.195.4` está associado a um domínio (ex: `exemplo.com.br`), use:

### Método 1: Usando Subfinder (Recomendado)

```bash
# Descobrir subdomínios
subfinder -d exemplo.com.br -o subdomains.txt

# Ver resultados
cat subdomains.txt
```

### Método 2: Usando Gobuster DNS

```bash
# Usando wordlist pequena (rápido)
gobuster dns \
  -d exemplo.com.br \
  -w /usr/share/seclists/Discovery/DNS/subdomains-top1million-5000.txt \
  -o gobuster-subdomains.txt

# Usando wordlist média (mais completo)
gobuster dns \
  -d exemplo.com.br \
  -w /usr/share/seclists/Discovery/DNS/subdomains-top1million-20000.txt \
  -o gobuster-subdomains-full.txt

# Usando wordlist grande (muito completo, mais lento)
gobuster dns \
  -d exemplo.com.br \
  -w /usr/share/seclists/Discovery/DNS/subdomains-top1million-110000.txt \
  -o gobuster-subdomains-complete.txt
```

### Método 3: Usando Reconner (Automático - Recomendado)

```bash
# Scan completo automatizado
reconner --target exemplo.com.br --output-dir ./scan-results

# Modo rápido
reconner --target exemplo.com.br --fast --output-dir ./scan-results

# Com proxy (Burp Suite)
reconner --target exemplo.com.br --proxy http://127.0.0.1:8080 --output-dir ./scan-results
```

---

## 🔍 Opção 2: Enumerar Diretórios/Arquivos no IP (Alternativa)

Se você quer descobrir **diretórios e arquivos** no servidor web `177.69.195.4`:

### Usando Gobuster DIR

```bash
# Scan básico (rápido)
gobuster dir \
  -u http://177.69.195.4/ \
  -w /usr/share/seclists/Discovery/Web-Content/common.txt \
  -o gobuster-dirs-basic.txt

# Scan médio (recomendado)
gobuster dir \
  -u http://177.69.195.4/ \
  -w /usr/share/seclists/Discovery/Web-Content/directory-list-2.3-medium.txt \
  -x php,html,txt,js \
  -o gobuster-dirs-medium.txt

# Scan completo (lento, muito completo)
gobuster dir \
  -u http://177.69.195.4/ \
  -w /usr/share/seclists/Discovery/Web-Content/directory-list-2.3-big.txt \
  -x php,html,txt,js,bak,old,zip,sql \
  -t 50 \
  -o gobuster-dirs-full.txt

# Scan com autenticação (se necessário)
gobuster dir \
  -u http://177.69.195.4/ \
  -w /usr/share/seclists/Discovery/Web-Content/common.txt \
  -U usuario \
  -P senha \
  -o gobuster-dirs-auth.txt
```

### Explicação dos Parâmetros

| Parâmetro | Descrição |
|-----------|-----------|
| `-u` | URL alvo |
| `-w` | Wordlist a usar |
| `-x` | Extensões de arquivo para testar |
| `-t` | Número de threads (padrão: 10) |
| `-o` | Arquivo de saída |
| `-U` | Usuário para autenticação básica |
| `-P` | Senha para autenticação básica |
| `-k` | Ignorar erros de certificado SSL |
| `-r` | Seguir redirecionamentos |
| `--proxy` | Usar proxy (ex: Burp Suite) |

---

## 📊 Wordlists Disponíveis no Seu Sistema

### Para Descoberta de Subdomínios (DNS)
```
/usr/share/seclists/Discovery/DNS/
├── subdomains-top1million-5000.txt      (5K - Rápido)
├── subdomains-top1million-20000.txt     (20K - Médio)
├── subdomains-top1million-110000.txt    (110K - Completo)
├── dns-Jhaddix.txt                      (Jhaddix's list)
├── combined_subdomains.txt              (Combinada)
└── fierce-hostlist.txt                  (Fierce)
```

### Para Enumeração de Diretórios (Web-Content)
```
/usr/share/seclists/Discovery/Web-Content/
├── common.txt                           (Pequena - Rápida)
├── directory-list-2.3-medium.txt        (Média - Recomendada)
├── directory-list-2.3-big.txt           (Grande - Completa)
├── raft-small-files.txt                 (Arquivos sensíveis)
├── raft-small-directories.txt           (Diretórios comuns)
└── quickhits.txt                        (Alvos rápidos)
```

---

## 🚀 Exemplos Práticos Completos

### Exemplo 1: Scan Rápido de Diretórios

```bash
# Criar diretório para resultados
mkdir -p ~/scans/177.69.195.4

# Scan rápido
gobuster dir \
  -u http://177.69.195.4/ \
  -w /usr/share/seclists/Discovery/Web-Content/common.txt \
  -x php,html,txt \
  -t 20 \
  -o ~/scans/177.69.195.4/quick-scan.txt

# Ver resultados
cat ~/scans/177.69.195.4/quick-scan.txt
```

### Exemplo 2: Scan Completo com Múltiplas Extensões

```bash
gobuster dir \
  -u http://177.69.195.4/ \
  -w /usr/share/seclists/Discovery/Web-Content/directory-list-2.3-medium.txt \
  -x php,html,txt,js,jsp,asp,aspx,bak,old,zip,tar.gz,sql,db \
  -t 50 \
  -k \
  -r \
  -o ~/scans/177.69.195.4/full-scan.txt \
  2>&1 | tee ~/scans/177.69.195.4/full-scan.log
```

### Exemplo 3: Scan com Proxy (Burp Suite)

```bash
# Primeiro, inicie o Burp Suite na porta 8080
# Depois execute:

gobuster dir \
  -u http://177.69.195.4/ \
  -w /usr/share/seclists/Discovery/Web-Content/common.txt \
  -x php,html \
  --proxy http://127.0.0.1:8080 \
  -o ~/scans/177.69.195.4/burp-scan.txt
```

### Exemplo 4: Buscar Arquivos Sensíveis

```bash
# Procurar por backups, configs, etc
gobuster dir \
  -u http://177.69.195.4/ \
  -w /usr/share/seclists/Discovery/Web-Content/raft-small-files.txt \
  -x bak,old,backup,zip,tar.gz,sql,db,conf,config,ini \
  -t 30 \
  -o ~/scans/177.69.195.4/sensitive-files.txt
```

---

## 📝 Script Automatizado Completo

Crie um arquivo `scan-ip.sh`:

```bash
#!/bin/bash

# Configurações
TARGET="http://177.69.195.4/"
OUTPUT_DIR="$HOME/scans/177.69.195.4"
SECLISTS="/usr/share/seclists"

# Cores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Scan Automatizado - $TARGET${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Criar diretório de saída
mkdir -p "$OUTPUT_DIR"

# 1. Scan rápido
echo -e "${YELLOW}[1/4] Executando scan rápido...${NC}"
gobuster dir \
  -u "$TARGET" \
  -w "$SECLISTS/Discovery/Web-Content/common.txt" \
  -x php,html,txt \
  -t 20 \
  -q \
  -o "$OUTPUT_DIR/01-quick-scan.txt"
echo -e "${GREEN}✓ Concluído: $OUTPUT_DIR/01-quick-scan.txt${NC}"
echo ""

# 2. Scan de diretórios
echo -e "${YELLOW}[2/4] Executando scan de diretórios...${NC}"
gobuster dir \
  -u "$TARGET" \
  -w "$SECLISTS/Discovery/Web-Content/directory-list-2.3-medium.txt" \
  -x php,html,txt,js \
  -t 50 \
  -q \
  -o "$OUTPUT_DIR/02-directories.txt"
echo -e "${GREEN}✓ Concluído: $OUTPUT_DIR/02-directories.txt${NC}"
echo ""

# 3. Scan de arquivos sensíveis
echo -e "${YELLOW}[3/4] Procurando arquivos sensíveis...${NC}"
gobuster dir \
  -u "$TARGET" \
  -w "$SECLISTS/Discovery/Web-Content/raft-small-files.txt" \
  -x bak,old,backup,zip,tar.gz,sql,db,conf,config \
  -t 30 \
  -q \
  -o "$OUTPUT_DIR/03-sensitive-files.txt"
echo -e "${GREEN}✓ Concluído: $OUTPUT_DIR/03-sensitive-files.txt${NC}"
echo ""

# 4. Resumo
echo -e "${YELLOW}[4/4] Gerando resumo...${NC}"
{
  echo "=========================================="
  echo "  RESUMO DO SCAN"
  echo "=========================================="
  echo "Alvo: $TARGET"
  echo "Data: $(date)"
  echo ""
  echo "Resultados encontrados:"
  echo ""
  echo "--- Scan Rápido ---"
  grep -E "Status: 200|Status: 301|Status: 302" "$OUTPUT_DIR/01-quick-scan.txt" 2>/dev/null || echo "Nenhum resultado"
  echo ""
  echo "--- Diretórios ---"
  grep -E "Status: 200|Status: 301|Status: 302" "$OUTPUT_DIR/02-directories.txt" 2>/dev/null | head -20 || echo "Nenhum resultado"
  echo ""
  echo "--- Arquivos Sensíveis ---"
  grep -E "Status: 200" "$OUTPUT_DIR/03-sensitive-files.txt" 2>/dev/null || echo "Nenhum resultado"
  echo ""
  echo "=========================================="
  echo "Todos os resultados salvos em: $OUTPUT_DIR"
  echo "=========================================="
} | tee "$OUTPUT_DIR/00-RESUMO.txt"

echo ""
echo -e "${GREEN}✓ Scan completo!${NC}"
echo -e "${BLUE}Resultados em: $OUTPUT_DIR${NC}"
```

**Para usar:**

```bash
# Tornar executável
chmod +x scan-ip.sh

# Executar
./scan-ip.sh
```

---

## ⚠️ Avisos Legais e Éticos

### 🔴 IMPORTANTE

1. **Autorização:** Apenas execute scans em sistemas que você possui ou tem permissão **EXPLÍCITA POR ESCRITO** para testar
2. **Ilegalidade:** Scans não autorizados são **ILEGAIS** e podem resultar em:
   - Processos criminais
   - Multas pesadas
   - Prisão
3. **Responsabilidade:** Você é **100% responsável** por suas ações
4. **IP de Teste:** O IP `177.69.195.4` que você forneceu — certifique-se de ter autorização!

### ✅ Ambientes Legais para Praticar

- **HackTheBox:** https://www.hackthebox.com/
- **TryHackMe:** https://tryhackme.com/
- **PentesterLab:** https://pentesterlab.com/
- **VulnHub:** https://www.vulnhub.com/
- **OWASP WebGoat:** https://owasp.org/www-project-webgoat/

---

## 🔧 Troubleshooting

### Problema: "Permission denied"
```bash
# Solução: Use sudo ou verifique permissões
sudo gobuster dir -u http://177.69.195.4/ -w /usr/share/seclists/Discovery/Web-Content/common.txt
```

### Problema: "Too many open files"
```bash
# Solução: Aumente o limite de arquivos abertos
ulimit -n 8192

# Ou reduza o número de threads
gobuster dir -u http://177.69.195.4/ -w wordlist.txt -t 10
```

### Problema: Scan muito lento
```bash
# Solução 1: Use wordlist menor
-w /usr/share/seclists/Discovery/Web-Content/common.txt

# Solução 2: Aumente threads
-t 50

# Solução 3: Reduza extensões
-x php,html
```

### Problema: Muitos erros 404
```bash
# Solução: Filtre status codes
gobuster dir -u http://177.69.195.4/ -w wordlist.txt -b 404,403
```

---

## 📚 Recursos Adicionais

### Documentação Oficial
- **Gobuster:** https://github.com/OJ/gobuster
- **SecLists:** https://github.com/danielmiessler/SecLists
- **Subfinder:** https://github.com/projectdiscovery/subfinder

### Tutoriais Recomendados
- **Gobuster Tutorial:** https://www.hackingarticles.in/comprehensive-guide-on-gobuster-tool/
- **Web Enumeration:** https://book.hacktricks.xyz/pentesting/pentesting-web

### Comandos Úteis
```bash
# Ver todas as wordlists disponíveis
find /usr/share/seclists -name "*.txt" | wc -l

# Procurar wordlist específica
find /usr/share/seclists -name "*subdomain*"

# Ver tamanho das wordlists
du -h /usr/share/seclists/Discovery/DNS/*.txt

# Combinar wordlists
cat wordlist1.txt wordlist2.txt | sort -u > combined.txt
```

---

## 🎯 Próximos Passos

Depois de encontrar diretórios/arquivos:

1. **Analisar manualmente** cada resultado encontrado
2. **Testar vulnerabilidades** conhecidas (SQLi, XSS, etc.)
3. **Usar Burp Suite** para análise mais profunda
4. **Executar Nuclei** para scan de vulnerabilidades automatizado
5. **Documentar tudo** para o relatório final

---

**Criado por:** Reconner v2.0.0  
**Data:** 2026-05-12  
**Licença:** MIT
