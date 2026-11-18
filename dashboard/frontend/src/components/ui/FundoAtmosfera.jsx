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

import fumaca from '../../assets/fundo-fumaca.jpg'
import ChuvaBinaria from './ChuvaBinaria.jsx'

// Fundo único do site, uma camada fixa atrás de todas as páginas (por isso a rolagem parece uma
// tela só, sem emendas entre as seções): base noite com brilhos glaciais (.fundo-atmosfera), a
// fumaça gelada da mesma arte do banner, em tamanho natural nos dois cantos de baixo (.fumaca-lado), e a chuva de binários discreta em canvas.
// É só enfeite (aria-hidden, sem foco, sem cliques).
export default function FundoAtmosfera() {
  return (
    <div aria-hidden="true" className="fundo-atmosfera pointer-events-none fixed inset-0 -z-10 overflow-hidden">
      <div className="fumaca-lado fumaca-direita" style={{ '--imagem-fumaca': `url(${fumaca})` }}>
        <div className="fumaca-img" />
      </div>
      <div className="fumaca-lado fumaca-esquerda" style={{ '--imagem-fumaca': `url(${fumaca})` }}>
        <div className="fumaca-img" />
      </div>
      <ChuvaBinaria />
    </div>
  )
}
