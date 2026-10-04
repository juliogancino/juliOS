#!/bin/bash
export LC_ALL=C

OUT="e3_http_warm_30.csv"

echo "run,http_latency_ms,ollama_total_ms,load_ms,prompt_eval_ms,eval_ms,response" > "$OUT"

for i in $(seq 1 30); do
    START=$(date +%s%N)

    JSON=$(curl -s http://127.0.0.1:11434/api/generate \
      -H 'Content-Type: application/json' \
      -d '{"model":"qwen2.5-coder:1.5b","prompt":"Responde exactamente JULIOS_OK","stream":false}')

    END=$(date +%s%N)

    HTTP_MS=$(awk "BEGIN {printf \"%.3f\", ($END-$START)/1000000}")

    TOTAL=$(echo "$JSON" | grep -o '"total_duration":[0-9]*' | cut -d: -f2)
    LOAD=$(echo "$JSON" | grep -o '"load_duration":[0-9]*' | cut -d: -f2)
    PROMPT=$(echo "$JSON" | grep -o '"prompt_eval_duration":[0-9]*' | cut -d: -f2)
    EVAL=$(echo "$JSON" | grep -o '"eval_duration":[0-9]*' | cut -d: -f2)
    RESPONSE=$(echo "$JSON" | grep -o '"response":"[^"]*"' | cut -d'"' -f4)

    TOTAL_MS=$(awk "BEGIN {printf \"%.3f\", $TOTAL/1000000}")
    LOAD_MS=$(awk "BEGIN {printf \"%.3f\", $LOAD/1000000}")
    PROMPT_MS=$(awk "BEGIN {printf \"%.3f\", $PROMPT/1000000}")
    EVAL_MS=$(awk "BEGIN {printf \"%.3f\", $EVAL/1000000}")

    echo "$i,$HTTP_MS,$TOTAL_MS,$LOAD_MS,$PROMPT_MS,$EVAL_MS,$RESPONSE" | tee -a "$OUT"
done
