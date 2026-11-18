/*
 * C-Shield — Dashboard (terceira camada: lê o FIWARE)
 * GS 2026.2 "CitizenTech" · Edge Computing & Computer Systems · FIAP · Prof. Dr. Fábio H. Cabrini
 * Equipe Nexus Vision. Integrantes:
 *   Isac Nilton Fernandes de Oliveira — RM573282
 *   João Benedito de Oliveira Simplício — RM570206
 *   Leonardo Formigari — RM573291
 *   Luis Fernando — RM570959
 *   Nicolas Cinel — RM572940
 */

/**
 * Hook de polling: roda uma tarefa assíncrona agora e de novo `intervaloMs`
 * depois que ela TERMINA (cadeia de setTimeout). Assim as requisições nunca
 * se acumulam, mesmo quando o FIWARE demora até o timeout para responder.
 */
import { useEffect, useRef } from 'react';

export function usePolling(tarefa, intervaloMs) {
  // Guarda a versão mais recente da tarefa sem reiniciar o ciclo
  const tarefaAtual = useRef(tarefa);
  useEffect(() => {
    tarefaAtual.current = tarefa;
  }, [tarefa]);

  useEffect(() => {
    let ativo = true;
    let timer = null;
    async function ciclo() {
      try {
        await tarefaAtual.current();
      } catch (erro) {
        // A tarefa deveria tratar os próprios erros; isto só evita parar o ciclo
        console.error('usePolling: tarefa falhou', erro);
      } finally {
        if (ativo) {
          timer = setTimeout(ciclo, intervaloMs);
        }
      }
    }
    ciclo();
    return () => {
      ativo = false;
      clearTimeout(timer);
    };
  }, [intervaloMs]);
}
