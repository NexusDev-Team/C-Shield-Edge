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
 * Configuração do Vite (servidor de desenvolvimento e build) e do Vitest.
 *
 * O dashboard chama sempre caminhos relativos (/api/...). Aqui o Vite encaminha
 * esses caminhos para o backend FastAPI (proxy), então não há CORS nem
 * endereço do backend dentro do código. O destino vem de BACKEND_URL no
 * dashboard/frontend/.env; sem ela, usa o backend local na porta 8000.
 * Em produção, quem faz esse papel é o rewrite do vercel.json.
 * O Tailwind CSS v4 entra pelo plugin oficial do Vite.
 */
import { defineConfig, loadEnv } from 'vite'
import react from '@vitejs/plugin-react'
import tailwindcss from '@tailwindcss/vite'

export default defineConfig(({ mode }) => {
  const env = loadEnv(mode, process.cwd(), '')
  const proxy = { '/api': env.BACKEND_URL || 'http://localhost:8000' }
  return {
    plugins: [react(), tailwindcss()],
    server: { port: 5173, strictPort: true, proxy },
    preview: { proxy },
    test: { environment: 'node', include: ['src/**/*.test.js'] },
  }
})
